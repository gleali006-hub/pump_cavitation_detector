//final code that to employs the trained model and effectively do the anomaly detection on the esp32

#include <tesi_inferencing.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <math.h>

#define TAU 0.1 //error limit value
#define releMotopompa 5 //motorpump switchoff relay pin

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
/**
 * @brief      Copy raw feature data in out_ptr
 *             Function called by inference library
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The out pointer
 *
 * @return     0
 */
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
size_t feature_ix = 0;
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}
void print_inference_result(ei_impulse_result_t result);
/**
 * @brief      Arduino setup function
 */
void setup()
{
    Serial.begin(115200);
    pinMode(releMotopompa, OUTPUT);
    digitalWrite(releMotopompa, 1);
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");
    if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("No ADXL345 detected ... Check your wiring!");
    while(1);
  }
  accel.setRange(ADXL345_RANGE_16_G);
  accel.setDataRate(ADXL345_DATARATE_3200_HZ);
}
/**
 * @brief      Arduino main function
 */
 float x_pred, y_pred, z_pred;
 float deltaX,deltaY,deltaZ;
 float RMS, RMS_med;
 float RMS_window[50];
 int i=0;

void loop()
{
    sensors_event_t event; 
    accel.getEvent(&event);
    if(abs(event.acceleration.x)>150||abs(event.acceleration.y)>150||abs(event.acceleration.z)>100)
        continue;
    features[feature_ix++] = map(event.acceleration.x,-150,150,0,1);
    features[feature_ix++] = map(event.acceleration.y,-150,150,0,1);
    features[feature_ix++] = map(event.acceleration.z,-100,100,0,1);

    ei_printf("Edge Impulse standalone inferencing (Arduino)\n");

    if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
            EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
        delay(1000);
        return;
    }
    ei_impulse_result_t result = { 0 };
    // the features are stored into flash, and we don't want to load everything into RAM
    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;
    // esegui l'inferenza
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
    if (res != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", res);
        return;
    }
    // inference return code
    ei_printf("run_classifier returned: %d\r\n", res);
    print_inference_result(result);

    //elaborazione inferenza:
    x_pred=result.classification[0].value;
    y_pred=result.classification[1].value;
    z_pred=result.classification[2].value;
    //Serial.print(event.x);Serial.print(x_pred);Serial.print(event.y);Serial.print(y_pred);Serial.print(event.z);Serial.println(z_pred);
    deltaX=x_pred-event.acceleration.x;
    deltaY=y_pred-event.acceleration.y;
    deltaZ=z_pred-event.acceleration.z;
    RMS=sqrt((deltaX*deltaX+deltaY*deltaY+deltaZ*deltaZ)/3);

    RMS_window[i]=RMS;
    if(i==49){
        i=0;
        float somma=0;
        for(int t=0;t<49;t++){
            somma=somma+RMS[t];
        }
        RMS_med=somma/50;
        if(RMS_med>=TAU){ //anomaly detected->motorpump is switched off
            digitalWrite(releMotopompa, 0);
            Serial.println("WARNING: Anomaly detected->motorpump is switched off");
            }
        }
    
    else
        i++;
}//endloop


void print_inference_result(ei_impulse_result_t result) {
    // Print how long it took to perform inference
    ei_printf("Timing: DSP %d ms, inference %d ms, anomaly %d ms\r\n",
            result.timing.dsp,
            result.timing.classification,
            result.timing.anomaly);
    // Print the prediction results (object detection)
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    ei_printf("Object detection bounding boxes:\r\n");
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        if (bb.value == 0) {
            continue;
        }
        ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label,
                bb.value,
                bb.x,
                bb.y,
                bb.width,
                bb.height);
    }
    // Print the prediction results (classification)
#else
    ei_printf("Predictions:\r\n");
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ei_printf("  %s: ", ei_classifier_inferencing_categories[i]);
        ei_printf("%.5f\r\n", result.classification[i].value);
    }
#endif
    // Print anomaly result (if it exists)
#if EI_CLASSIFIER_HAS_ANOMALY
    ei_printf("Anomaly prediction: %.3f\r\n", result.anomaly);
#endif
#if EI_CLASSIFIER_HAS_VISUAL_ANOMALY
    ei_printf("Visual anomalies:\r\n");
    for (uint32_t i = 0; i < result.visual_ad_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.visual_ad_grid_cells[i];
        if (bb.value == 0) {
            continue;
        }
        ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label,
                bb.value,
                bb.x,
                bb.y,
                bb.width,
                bb.height);
    }
#endif
}
