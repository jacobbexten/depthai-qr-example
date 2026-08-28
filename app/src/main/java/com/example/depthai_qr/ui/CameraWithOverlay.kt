package com.example.depthai_qr.ui

import android.graphics.Bitmap
import android.graphics.Paint as AndroidPaint
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.nativeCanvas
import androidx.compose.ui.layout.ContentScale
import com.example.depthai_qr.model.QrOverlay

@Composable
fun CameraWithOverlay(
    bitmap: Bitmap,
    overlays: List<QrOverlay>,
    modifier: Modifier = Modifier
) {
    val aspect = bitmap.width.toFloat() / bitmap.height.toFloat()

    Box(
        modifier = modifier
            .fillMaxWidth()
            .aspectRatio(aspect)
    ) {
        Image(
            bitmap = bitmap.asImageBitmap(),
            contentDescription = null,
            modifier = Modifier.fillMaxWidth().aspectRatio(aspect),
            contentScale = ContentScale.FillBounds
        )

        Canvas(modifier = Modifier.fillMaxWidth().aspectRatio(aspect)) {
            val textPaint = AndroidPaint().apply {
                color = android.graphics.Color.RED
                textSize = 42f
                isAntiAlias = true
                style = AndroidPaint.Style.FILL
            }

            val strokePaint = AndroidPaint().apply {
                color = android.graphics.Color.BLACK
                textSize = 42f
                isAntiAlias = true
                style = AndroidPaint.Style.STROKE
                strokeWidth = 6f
            }

            overlays.forEach { overlay ->
                val left = overlay.xmin * size.width
                val top = overlay.ymin * size.height
                val right = overlay.xmax * size.width
                val bottom = overlay.ymax * size.height

                drawRect(
                    color = Color.Red,
                    topLeft = Offset(left, top),
                    size = Size(right - left, bottom - top),
                    style = Stroke(width = 4f)
                )

                val distanceLabel = overlay.distanceFeet?.let { String.format("%.1f ft", it) } ?: "-- ft"
                val label = overlay.text?.let { "$it  |  $distanceLabel" } ?: distanceLabel

                val textX = left
                val textY = (top - 12f).coerceAtLeast(42f)

                drawContext.canvas.nativeCanvas.apply {
                    drawText(label, textX, textY, strokePaint)
                    drawText(label, textX, textY, textPaint)
                }
            }
        }
    }
}
