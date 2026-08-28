package com.example.depthai_qr.model

data class QrOverlay(
    val xmin: Float,
    val ymin: Float,
    val xmax: Float,
    val ymax: Float,
    val confidence: Float,
    val distanceFeet: Double?,
    val text: String?
)
