package com.example.depthai_qr

import android.graphics.Bitmap
import android.os.Bundle
import android.os.Handler
import android.os.HandlerThread
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import com.google.mlkit.vision.barcode.BarcodeScannerOptions
import com.google.mlkit.vision.barcode.BarcodeScanning
import com.google.mlkit.vision.barcode.common.Barcode
import com.google.mlkit.vision.common.InputImage
import com.example.depthai_qr.model.QrDetection
import com.example.depthai_qr.model.QrOverlay
import com.example.depthai_qr.ui.CameraWithOverlay

class MainActivity : ComponentActivity() {
    private val bridge = DepthaiBridge()

    private val frameHandlerThread = HandlerThread("FrameThread").apply { start() }
    private val frameHandler = Handler(frameHandlerThread.looper)

    private var running = true

    private var rgbBitmap by mutableStateOf<Bitmap?>(null)
    private var depthBitmap by mutableStateOf<Bitmap?>(null)
    private var qrOverlays by mutableStateOf<List<QrOverlay>>(emptyList())
    private var decodeInFlight = false

    private val qrScanner by lazy {
        val options = BarcodeScannerOptions.Builder()
            .setBarcodeFormats(Barcode.FORMAT_QR_CODE)
            .build()

        BarcodeScanning.getClient(options)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val blobPath = copyAssetToFilesDir("qr_code_detection_384x384.blob")
        bridge.startDevice(blobPath)

        setContent {
            MaterialTheme {
                val rgb = rgbBitmap
                if (rgb != null) {
                    CameraWithOverlay(
                        bitmap = rgb,
                        overlays = qrOverlays,
                        modifier = Modifier.fillMaxWidth()
                    )
                } else {
                    Text("Waiting for RGB frame...")
                }
            }
        }

        startFrameLoop()
    }

    private fun startFrameLoop() {
        frameHandler.post(object : Runnable {
            override fun run() {
                if (running) {
                    val rgb = bridge.getRgbFrame()
                    val rgbWidth = bridge.getRgbWidth()
                    val rgbHeight = bridge.getRgbHeight()

                    val bitmap = if (
                        rgb != null &&
                        rgb.isNotEmpty() &&
                        rgbWidth > 0 &&
                        rgbHeight > 0 &&
                        rgb.size == rgbWidth * rgbHeight
                    ) {
                        Bitmap.createBitmap(
                            rgb,
                            rgbWidth,
                            rgbHeight,
                            Bitmap.Config.ARGB_8888
                        )
                    } else null

                    if (bitmap != null) {
                        runOnUiThread {
                            rgbBitmap = bitmap
                        }
                    }

                    val detections = parseQrDetections(bridge.getQrDetections())

                    if (bitmap != null && detections.isNotEmpty()) {
                        val depth = bridge.getDepthFrame()
                        val depthW = bridge.getDepthWidth()
                        val depthH = bridge.getDepthHeight()

                        val baseOverlays = detections.map { det ->
                            QrOverlay(
                                xmin = det.xmin,
                                ymin = det.ymin,
                                xmax = det.xmax,
                                ymax = det.ymax,
                                confidence = det.confidence,
                                distanceFeet = if (
                                    depth != null &&
                                    depthW > 0 &&
                                    depthH > 0 &&
                                    depth.size == depthW * depthH
                                ) {
                                    medianDepthFeet(depth, depthW, depthH, det)
                                } else {
                                    null
                                },
                                text = null
                            )
                        }

                        runOnUiThread {
                            qrOverlays = baseOverlays
                        }

                        if (!decodeInFlight) {
                            decodeInFlight = true
                            val firstDet = detections.first()

                            decodeQrFromDetection(bitmap, firstDet) { decoded ->
                                runOnUiThread {
                                    qrOverlays = baseOverlays.mapIndexed { index, overlay ->
                                        if (index == 0) overlay.copy(text = decoded) else overlay
                                    }
                                    decodeInFlight = false
                                }
                            }
                        }
                    } else {
                        runOnUiThread {
                            qrOverlays = emptyList()
                        }
                    }

                    frameHandler.postDelayed(this, 30)
                }
            }
        })
    }

    private fun parseQrDetections(data: FloatArray?): List<QrDetection> {
        if (data == null || data.isEmpty()) return emptyList()
        val result = mutableListOf<QrDetection>()
        var i = 0
        while (i + 4 < data.size) {
            result += QrDetection(
                xmin = data[i],
                ymin = data[i + 1],
                xmax = data[i + 2],
                ymax = data[i + 3],
                confidence = data[i + 4]
            )
            i += 5
        }
        return result
    }

    private fun medianDepthFeet(
        depth: ShortArray,
        width: Int,
        height: Int,
        det: QrDetection,
    ): Double? {
        val left = (det.xmin * width).toInt().coerceIn(0, width - 1)
        val top = (det.ymin * height).toInt().coerceIn(0, height - 1)
        val right = (det.xmax * width).toInt().coerceIn(0, width - 1)
        val bottom = (det.ymax * height).toInt().coerceIn(0, height - 1)

        if (right <= left || bottom <= top) return null

        // shrink ROI to center 50%
        val roiW = right - left
        val roiH = bottom - top
        val insetX = roiW / 4
        val insetY = roiH / 4

        val sLeft = (left + insetX).coerceIn(0, width - 1)
        val sTop = (top + insetY).coerceIn(0, height - 1)
        val sRight = (right - insetX).coerceIn(0, width - 1)
        val sBottom = (bottom - insetY).coerceIn(0, height - 1)

        if (sRight <= sLeft || sBottom <= sTop) return null

        val values = ArrayList<Int>()
        for (y in sTop until sBottom) {
            val row = y * width
            for (x in sLeft until sRight) {
                val mm = depth[row + x].toInt() and 0xFFFF
                if (mm > 0) values += mm
            }
        }

        if (values.isEmpty()) return null

        values.sort()
        val medianMm = values[values.size / 2]
        return medianMm / 304.8
    }

    private fun decodeQrFromDetection(
        bitmap: Bitmap,
        det: QrDetection,
        onResult: (String?) -> Unit,
    ) {
        val left = (det.xmin * bitmap.width).toInt().coerceIn(0, bitmap.width - 1)
        val top = (det.ymin * bitmap.height).toInt().coerceIn(0, bitmap.height - 1)
        val right = (det.xmax * bitmap.width).toInt().coerceIn(0, bitmap.width - 1)
        val bottom = (det.ymax * bitmap.height).toInt().coerceIn(0, bitmap.height - 1)

        if (right <= left || bottom <= top) {
            onResult(null)
            return
        }

        val padX = ((right - left) * 0.02f).toInt()
        val padY = ((bottom - top) * 0.02f).toInt()

        val cropLeft = (left - padX).coerceAtLeast(0)
        val cropTop = (top - padY).coerceAtLeast(0)
        val cropRight = (right + padX).coerceAtMost(bitmap.width)
        val cropBottom = (bottom + padY).coerceAtMost(bitmap.height)

        val cropWidth = cropRight - cropLeft
        val cropHeight = cropBottom - cropTop
        if (cropWidth <= 0 || cropHeight <= 0) {
            onResult(null)
            return
        }

        val cropped = Bitmap.createBitmap(bitmap, cropLeft, cropTop, cropWidth, cropHeight)
        val image = InputImage.fromBitmap(cropped, 0)

        qrScanner.process(image)
            .addOnSuccessListener { barcodes ->
                onResult(barcodes.firstOrNull()?.rawValue)
            }
            .addOnFailureListener {
                onResult(null)
            }
    }

    private fun copyAssetToFilesDir(assetName: String): String {
        val outFile = java.io.File(filesDir, assetName)
        if (!outFile.exists()) {
            assets.open(assetName).use { input ->
                outFile.outputStream().use { output ->
                    input.copyTo(output)
                }
            }
        }
        return outFile.absolutePath
    }

    override fun onDestroy() {
        running = false
        frameHandlerThread.quitSafely()
        bridge.stopDevice()
        super.onDestroy()
    }
}
