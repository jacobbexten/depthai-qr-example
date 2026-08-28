package com.example.depthai_qr.model

data class QrDetection(
    val xmin: Float,
    val ymin: Float,
    val xmax: Float,
    val ymax: Float,
    val confidence: Float
)
