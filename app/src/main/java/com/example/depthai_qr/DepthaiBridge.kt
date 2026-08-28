package com.example.depthai_qr

class DepthaiBridge {

    companion object {
        init {
            System.loadLibrary("qrtest")
        }
    }

    external fun startDevice(blobPath: String): Boolean
    external fun getRgbFrame(): IntArray?
    external fun getRgbPreviewFrame(): IntArray?
    external fun getRgbWidth(): Int
    external fun getRgbHeight(): Int
    external fun getDepthFrame(): ShortArray?
    external fun getDepthWidth(): Int
    external fun getDepthHeight(): Int
    external fun getQrDetections(): FloatArray?
    external fun stopDevice()
}
