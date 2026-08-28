#include <jni.h>
#include <android/log.h>
#include <memory>
#include <libusb-1.0/libusb.h>
#include <mutex>
#include <string>

#include <depthai/depthai.hpp>

using namespace std;

#define LOG_TAG "DEPTHAI"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

std::shared_ptr<dai::Device> device;
std::shared_ptr<dai::DataOutputQueue> qRgb;
std::shared_ptr<dai::DataOutputQueue> qRgbPreview;
std::shared_ptr<dai::DataOutputQueue> qDepth;
std::shared_ptr<dai::DataOutputQueue> qQrDet;

static int gRgbWidth = 0;
static int gRgbHeight = 0;
static int gDepthWidth = 0;
static int gDepthHeight = 0;

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_ifg_qrtest_DepthaiBridge_startDevice(JNIEnv *env, jobject /*thiz*/, jstring blobPathJ) {
    try {
        int r = libusb_set_option(nullptr, LIBUSB_OPTION_ANDROID_JNIENV, env);
        LOGD("libusb_set_option ANDROID_JNIENV (start): %d", r);
        dai::Pipeline pipeline;

        auto camRgb = pipeline.create<dai::node::ColorCamera>();

        auto xoutRgb = pipeline.create<dai::node::XLinkOut>();
        xoutRgb->setStreamName("rgb");
        camRgb->video.link(xoutRgb->input);

        auto xoutRgbPreview = pipeline.create<dai::node::XLinkOut>();
        xoutRgbPreview->setStreamName("preview");

        camRgb->setPreviewSize(300, 300);
        camRgb->setBoardSocket(dai::CameraBoardSocket::RGB);
        camRgb->setResolution(dai::ColorCameraProperties::SensorResolution::THE_1080_P);
        camRgb->setInterleaved(false);
        camRgb->setColorOrder(dai::ColorCameraProperties::ColorOrder::RGB);
        camRgb->preview.link(xoutRgbPreview->input);

        auto monoLeft = pipeline.create<dai::node::MonoCamera>();
        auto monoRight = pipeline.create<dai::node::MonoCamera>();
        auto stereo = pipeline.create<dai::node::StereoDepth>();
        auto xoutDepth = pipeline.create<dai::node::XLinkOut>();
        xoutDepth->setStreamName("depth");

        monoLeft->setResolution(dai::MonoCameraProperties::SensorResolution::THE_400_P);
        monoLeft->setBoardSocket(dai::CameraBoardSocket::LEFT);

        monoRight->setResolution(dai::MonoCameraProperties::SensorResolution::THE_400_P);
        monoRight->setBoardSocket(dai::CameraBoardSocket::RIGHT);

        stereo->setDefaultProfilePreset(dai::node::StereoDepth::PresetMode::HIGH_DENSITY);
        stereo->setLeftRightCheck(true);
        stereo->setExtendedDisparity(false);
        stereo->setSubpixel(false);

        stereo->setDepthAlign(dai::CameraBoardSocket::RGB);

        monoLeft->out.link(stereo->left);
        monoRight->out.link(stereo->right);
        stereo->depth.link(xoutDepth->input);

        const char *blobPathC = env->GetStringUTFChars(blobPathJ, nullptr);
        std::string blobPath(blobPathC ? blobPathC : "");
        env->ReleaseStringUTFChars(blobPathJ, blobPathC);

        auto xoutQrDet = pipeline.create<dai::node::XLinkOut>();
        xoutQrDet->setStreamName("qrDet");

        auto manip = pipeline.create<dai::node::ImageManip>();
        manip->initialConfig.setResize(384, 384);
        manip->initialConfig.setFrameType(dai::ImgFrame::Type::GRAY8);

        auto qrDet = pipeline.create<dai::node::MobileNetDetectionNetwork>();
        qrDet->setBlobPath(blobPath);
        qrDet->setConfidenceThreshold(0.30f);
        qrDet->input.setBlocking(false);

        camRgb->video.link(manip->inputImage);
        manip->out.link(qrDet->input);
        qrDet->out.link(xoutQrDet->input);

        device = make_shared<dai::Device>(pipeline, dai::UsbSpeed::SUPER);

        qRgb = device->getOutputQueue("rgb", 4, false);
        qRgbPreview = device->getOutputQueue("preview", 4, false);
        qDepth = device->getOutputQueue("depth", 4, false);
        qQrDet = device->getOutputQueue("qrDet", 4, false);
        if (!qRgb || !qRgbPreview || !qDepth || !qQrDet) {
            LOGE("One or more output queues are null");
            qRgb.reset();
            qDepth.reset();
            device.reset();
            return JNI_FALSE;
        }

        LOGD("DepthAI RGB + depth pipeline started successfully");
        return JNI_TRUE;

    } catch (const std::exception &e) {
        LOGE("startDevice failed: %s", e.what());
        qRgb.reset();
        qDepth.reset();
        device.reset();
        return JNI_FALSE;
    }
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_ifg_qrtest_DepthaiBridge_getRgbFrame(JNIEnv *env, jobject /*thiz*/) {
    if (!qRgb) {
        LOGE("getFullRgbFrame called with null qRgbFull");
        return nullptr;
    }

    auto inRgb = qRgb->tryGet<dai::ImgFrame>();
    if (!inRgb) return nullptr;

    const int width = static_cast<int>(inRgb->getWidth());
    const int height = static_cast<int>(inRgb->getHeight());
    const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);

    gRgbWidth = width;
    gRgbHeight = height;

    const auto &data = inRgb->getData();
    if (data.size() < pixelCount * 3) {
        LOGE("Unexpected full RGB frame size. width=%d height=%d data=%zu", width, height,
             data.size());
        return nullptr;
    }

    const uint8_t *rPlane = data.data();
    const uint8_t *gPlane = data.data() + pixelCount;
    const uint8_t *bPlane = data.data() + pixelCount * 2;

    std::vector<jint> pixels(pixelCount);
    for (size_t i = 0; i < pixelCount; ++i) {
        pixels[i] =
                (0xFF << 24) |
                (static_cast<jint>(rPlane[i]) << 16) |
                (static_cast<jint>(gPlane[i]) << 8) |
                static_cast<jint>(bPlane[i]);
    }

    jintArray result = env->NewIntArray(static_cast<jsize>(pixelCount));
    if (!result) return nullptr;

    env->SetIntArrayRegion(result, 0, static_cast<jsize>(pixelCount), pixels.data());
    return result;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_ifg_qrtest_DepthaiBridge_getRgbWidth(JNIEnv *, jobject) {
    return gRgbWidth;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_ifg_qrtest_DepthaiBridge_getRgbHeight(JNIEnv *, jobject) {
    return gRgbHeight;
}

extern "C" JNIEXPORT jintArray JNICALL
Java_com_ifg_qrtest_DepthaiBridge_getRgbPreviewFrame(JNIEnv *env, jobject /* this */) {
    auto inRgbPreview = qRgbPreview->tryGet<dai::ImgFrame>();

    if (!inRgbPreview) return nullptr;

    const int width = static_cast<int>(inRgbPreview->getWidth());
    const int height = static_cast<int>(inRgbPreview->getHeight());
    const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);

    const auto &data = inRgbPreview->getData();
    if (data.size() < pixelCount * 3) {
        LOGE("Unexpected frame size. width=%d height=%d data=%zu", width, height, data.size());
        return nullptr;
    }

    const uint8_t *rPlane = data.data();
    const uint8_t *gPlane = data.data() + pixelCount;
    const uint8_t *bPlane = data.data() + (pixelCount * 2);

    std::vector<jint> pixels(pixelCount);

    for (size_t i = 0; i < pixelCount; ++i) {
        const uint8_t r = rPlane[i];
        const uint8_t g = gPlane[i];
        const uint8_t b = bPlane[i];

        pixels[i] =
                (0xFF << 24) |
                (static_cast<jint>(r) << 16) |
                (static_cast<jint>(g) << 8) |
                static_cast<jint>(b);
    }

    jintArray result = env->NewIntArray(static_cast<jsize>(pixelCount));
    if (!result) {
        LOGE("Failed to allocate jintArray");
        return nullptr;
    }

    env->SetIntArrayRegion(result, 0, static_cast<jsize>(pixelCount), pixels.data());
    LOGD("Returning frame %dx%d", width, height);

    return result;
}

extern "C"
JNIEXPORT jshortArray JNICALL
Java_com_ifg_qrtest_DepthaiBridge_getDepthFrame(JNIEnv *env, jobject /*thiz*/) {
    if (!qDepth) {
        LOGE("getDepthFrame called with null qDepth");
        return nullptr;
    }

    auto inDepth = qDepth->tryGet<dai::ImgFrame>();
    if (!inDepth) return nullptr;

    const int width = static_cast<int>(inDepth->getWidth());
    const int height = static_cast<int>(inDepth->getHeight());
    const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);

    gDepthWidth = width;
    gDepthHeight = height;

    const auto &bytes = inDepth->getData();
    if (bytes.size() < pixelCount * sizeof(uint16_t)) {
        LOGE("Unexpected depth frame size. width=%d height=%d bytes=%zu", width, height,
             bytes.size());
        return nullptr;
    }

    std::vector<jshort> depth(pixelCount);
    std::memcpy(depth.data(), bytes.data(), pixelCount * sizeof(uint16_t));

    jshortArray result = env->NewShortArray(static_cast<jsize>(pixelCount));
    if (!result) return nullptr;

    env->SetShortArrayRegion(result, 0, static_cast<jsize>(pixelCount), depth.data());
    LOGD("Returning depth frame %dx%d", width, height);
    return result;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_ifg_qrtest_DepthaiBridge_getDepthWidth(JNIEnv * /*env*/, jobject /*thiz*/) {
    return gDepthWidth;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_ifg_qrtest_DepthaiBridge_getDepthHeight(JNIEnv * /*env*/, jobject /*thiz*/) {
    return gDepthHeight;
}

extern "C"
JNIEXPORT jfloatArray JNICALL
Java_com_ifg_qrtest_DepthaiBridge_getQrDetections(JNIEnv *env, jobject /*thiz*/) {
    if (!qQrDet) {
        LOGE("getQrDetections called with null qQrDet");
        return nullptr;
    }

    auto detMsg = qQrDet->tryGet<dai::ImgDetections>();
    if (!detMsg) return nullptr;

    const auto &dets = detMsg->detections;
    if (dets.empty()) return nullptr;

    std::vector<float> flat;
    flat.reserve(dets.size() * 5);

    for (const auto &d: dets) {
        flat.push_back(d.xmin);
        flat.push_back(d.ymin);
        flat.push_back(d.xmax);
        flat.push_back(d.ymax);
        flat.push_back(d.confidence);
    }

    jfloatArray arr = env->NewFloatArray(static_cast<jsize>(flat.size()));
    if (!arr) return nullptr;

    env->SetFloatArrayRegion(arr, 0, static_cast<jsize>(flat.size()), flat.data());
    return arr;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ifg_qrtest_DepthaiBridge_stopDevice(JNIEnv * /*env*/, jobject /*thiz*/) {
    qRgb.reset();
    qRgbPreview.reset();
    qDepth.reset();
    qQrDet.reset();
    device.reset();
    gRgbWidth = 0;
    gRgbHeight = 0;
    gDepthWidth = 0;
    gDepthHeight = 0;
    LOGD("DepthAI native state cleared");
}
