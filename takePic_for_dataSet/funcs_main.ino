void setup_camera(){
  CamErr err = theCamera.begin(1, CAM_VIDEO_FPS_15, width, height, pixfmt);
  if (err != CAM_ERR_SUCCESS) {
    Serial.println("camera begin err: " + String(err));
    return;
  }
  err = theCamera.startStreaming(true, CamCB);
  if (err != CAM_ERR_SUCCESS) {
    Serial.println("start streaming err: " + String(err));
    return;
  }
}

uint16_t* getImageData(CamImage img){
    if (!img.isAvailable()) {
        Serial.println("img is not available");
        return;
    }
    uint16_t* buf = (uint16_t*)img.getImgBuff();

    return buf;
}