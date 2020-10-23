#include <vector>
#include <string>
#include <stdio.h>
#include "c_api.h"
#include "delegate.h"


//std::unique_ptr<TfLiteInterpreter> interpreter;
//TfLiteInterpreter interpreter;

enum class BodyPart {
   NOSE,
   LEFT_EYE,
   RIGHT_EYE,
   LEFT_EAR,
   RIGHT_EAR,
   LEFT_SHOULDER,
   RIGHT_SHOULDER,
   LEFT_ELBOW,
   RIGHT_ELBOW,
   LEFT_WRIST,
   RIGHT_WRIST,
   LEFT_HIP,
   RIGHT_HIP,
   LEFT_KNEE,
   RIGHT_KNEE,
   LEFT_ANKLE,
   RIGHT_ANKLE
};


class Position {
    TfLiteModel* model;
    float x;
    float y;
};

class KeyPoint {
  BodyPart bodyPart;
  Position position;
  float score;
};

class Person {
  std::vector<KeyPoint> keyPoints;
  float score;
};

enum class Device {
      CPU,
      NNAPI,
      GPU
    };


class Posenet  {
    const char* filename;
    Device device;
    long lastInferenceTimeNanos = -1;

    //the model
    TfLiteModel* model;

    //the options for the interpreter (like settings)
    TfLiteInterpreterOptions* options;

    //interpreter for tflite model
    TfLiteInterpreter* interpreter;

    //private GpuDelegate

    int NUM_LITE_THREADS = 4;


    Posenet(const char* pFilename, Device pDevice) {
        this->filename = pFilename;
        this->device = pDevice;
    }


    //set up the Tensorflow inference interpreter
    TfLiteInterpreter* getInterpreter() {

        //get the Posenet Interpreter instance if it already exists
        if (interpreter != NULL) {
            return interpreter;
        }

        //otherwise we need to create a new interpreter

        //create interpreter options
        options = TfLiteInterpreterOptionsCreate();

        //set number of threads to use
        TfLiteInterpreterOptionsSetNumThreads(options, NUM_LITE_THREADS);

        if (this->device == Device::GPU) {
            //set up GPU delegate
            /*
            gpuDelegate = GpuDelegate()
            options.addDelegate(gpuDelegate)*/
        }
        else if (device == Device::NNAPI) {
            //what to do here?
            //Device.NNAPI -> options.setUseNNAPI(true)
        }

        //Set up interpreter.
        model = TfLiteModelCreateFromFile(this->filename);


        //customize the gpu delegate
        const TfLiteGpuDelegateOptionsV2 gpuDelegate = {
                .is_precision_loss_allowed = false,
                .inference_preference = TFLITE_GPU_INFERENCE_PREFERENCE_FAST_SINGLE_ANSWER,
                .inference_priority1 = TFLITE_GPU_INFERENCE_PRIORITY_MIN_LATENCY,
                .inference_priority2 = TFLITE_GPU_INFERENCE_PRIORITY_AUTO,
                .inference_priority3 = TFLITE_GPU_INFERENCE_PRIORITY_AUTO
        }; //TfLiteGpuDelegateOptionsV2Default() doesn't fix the error either


        TfLiteDelegate* delegate = TfLiteGpuDelegateV2Create(&gpuDelegate);

        //TfLiteDelegate* delegate = TfLiteGpuDelegateV2Default(&gpuDelegate);


        TfLiteInterpreterOptionsAddDelegate(options, delegate);


        TfLiteInterpreter* interpreter = TfLiteInterpreterCreate(model, options);
        TfLiteInterpreterAllocateTensors(interpreter);


        TfLiteTensor* input_tensor = TfLiteInterpreterGetInputTensor(interpreter, 0);
        const TfLiteTensor* output_tensor = TfLiteInterpreterGetOutputTensor(interpreter, 0);
        //TfLiteStatus from_status = TfLiteTensorCopyFromBuffer(input_tensor, input_data, TfLiteTensorByteSize(input_tensor));


        TfLiteStatus interpreter_invoke_status = TfLiteInterpreterInvoke(interpreter);


        //TfLiteStatus to_status = TfLiteTensorCopyToBuffer(output_tensor, output_data, TfLiteTensorByteSize(output_tensor));


        //if (interpreter->ModifyGraphWithDelegate(delegate) != kTfLiteOk) return;

/*
        ops::builtin::BuiltinOpResolver op_resolver;

        TfLiteInterpreter* interpreter;

        InterpreterBuilder(*model, op_resolver)(&interpreter);
*/

        return interpreter;
}

    void close() {
        //delete the interpreter we made
        TfLiteInterpreterDelete(interpreter);
        TfLiteInterpreterOptionsDelete(options);
        TfLiteModelDelete(model);
    }
/*
  //clean up the interpreter and possibly the gpuDelegate
  override fun close() {
    interpreter?.close()
    interpreter = null
    gpuDelegate?.close()
    gpuDelegate = null
  }

 //Returns value within [0,1].
  private fun sigmoid(x: Float): Float {
    return (1.0f / (1.0f + exp(-x)))
  }


   //Scale the image to a byteBuffer of [-1,1] values.

  private fun initInputArray(bitmap: Bitmap): ByteBuffer {
    val bytesPerChannel = 4
    val inputChannels = 3
    val batchSize = 1

    //allocate a ByteBuffer for all 3 input channels (for each channel allocate space for the entire bitmap)
    val inputBuffer = ByteBuffer.allocateDirect(batchSize * bytesPerChannel * bitmap.height * bitmap.width * inputChannels)

    inputBuffer.order(ByteOrder.nativeOrder())

    inputBuffer.rewind()

    val mean = 128.0f
    val std = 128.0f

    //create an int array that's size of the bitmap
    val intValues = IntArray(bitmap.width * bitmap.height)

    //get all bitmap pixels and store them in intValues
    bitmap.getPixels(intValues, 0, bitmap.width, 0, 0, bitmap.width, bitmap.height)

    //put one float into the ByteBuffer for I'm guessing each input channel
    for (pixelValue in intValues) {
      inputBuffer.putFloat(((pixelValue shr 16 and 0xFF) - mean) / std)
      inputBuffer.putFloat(((pixelValue shr 8 and 0xFF) - mean) / std)
      inputBuffer.putFloat(((pixelValue and 0xFF) - mean) / std)
    }

    return inputBuffer
  }

  // Preload and memory map the model file, returning a MappedByteBuffer containing the model.
  private fun loadModelFile(path: String, context: Context): MappedByteBuffer {
    //open up the file
    val fileDescriptor = context.assets.openFd(path)

    //start an input stream to write into the file
    val inputStream = FileInputStream(fileDescriptor.fileDescriptor)

    return inputStream.channel.map(FileChannel.MapMode.READ_ONLY, fileDescriptor.startOffset, fileDescriptor.declaredLength)
  }


  //Initializes an outputMap of 1 * x * y * z FloatArrays for the model processing to populate.

  private fun initOutputMap(interpreter: Interpreter): HashMap<Int, Any> {
    val outputMap = HashMap<Int, Any>()

    // 1 * 9 * 9 * 17 contains heatmaps
    val heatmapsShape = interpreter.getOutputTensor(0).shape()
    outputMap[0] = Array(heatmapsShape[0]) {
      Array(heatmapsShape[1]) {
        Array(heatmapsShape[2]) { FloatArray(heatmapsShape[3]) }
      }
    }

    // 1 * 9 * 9 * 34 contains offsets
    val offsetsShape = interpreter.getOutputTensor(1).shape()
    outputMap[1] = Array(offsetsShape[0]) {
      Array(offsetsShape[1]) { Array(offsetsShape[2]) { FloatArray(offsetsShape[3]) } }
    }

    // 1 * 9 * 9 * 32 contains forward displacements
    val displacementsFwdShape = interpreter.getOutputTensor(2).shape()
    outputMap[2] = Array(offsetsShape[0]) {
      Array(displacementsFwdShape[1]) {
        Array(displacementsFwdShape[2]) { FloatArray(displacementsFwdShape[3]) }
      }
    }

    //1 * 9 * 9 * 32 contains backward displacements
    val displacementsBwdShape = interpreter.getOutputTensor(3).shape()
    outputMap[3] = Array(displacementsBwdShape[0]) {
      Array(displacementsBwdShape[1]) {
        Array(displacementsBwdShape[2]) { FloatArray(displacementsBwdShape[3]) }
      }
    }

    return outputMap
  }


    Estimates the pose for a single person.
    args:
         bitmap: image bitmap of frame that should be processed
   returns:
        person: a Person object containing data about keypoint locations and confidence scores

  @Suppress("UNCHECKED_CAST")
  fun estimateSinglePose(bitmap: Bitmap): Person {
    val estimationStartTimeNanos = SystemClock.elapsedRealtimeNanos()
    val inputArray = arrayOf(initInputArray(bitmap))

    //print out how long scaling took
    //Log.i("posenet", String.format("Scaling to [-1,1] took %.2f ms", 1.0f * (SystemClock.elapsedRealtimeNanos() - estimationStartTimeNanos) / 1_000_000))

    val outputMap = initOutputMap(getInterpreter())

    //get the elapsed time since system boot
    val inferenceStartTimeNanos = SystemClock.elapsedRealtimeNanos()

    //from https://www.tensorflow.org/lite/guide/inference: each entry in inputArray corresponds to an input tensor and
    //outputMap maps indices of output tensors to the corresponding output data.
    getInterpreter().runForMultipleInputsOutputs(inputArray, outputMap)

    //get the elapsed time since system boot again, and subtract the first split we took to find how long running the model took
    lastInferenceTimeNanos = SystemClock.elapsedRealtimeNanos() - inferenceStartTimeNanos

    //print out how long the interpreter took
    //Log.i("posenet", String.format("Interpreter took %.2f ms", 1.0f * lastInferenceTimeNanos / 1_000_000))

    val heatmaps = outputMap[0] as Array<Array<Array<FloatArray>>>
    val offsets = outputMap[1] as Array<Array<Array<FloatArray>>>

    val height = heatmaps[0].size
    val width = heatmaps[0][0].size
    val numKeypoints = heatmaps[0][0][0].size

    // Finds the (row, col) locations of where the keypoints are most likely to be.
    val keypointPositions = Array(numKeypoints) { Pair(0, 0) }
    for (keypoint in 0 until numKeypoints) {
      var maxVal = heatmaps[0][0][0][keypoint]
      var maxRow = 0
      var maxCol = 0
      for (row in 0 until height) {
        for (col in 0 until width) {
          if (heatmaps[0][row][col][keypoint] > maxVal) {
            maxVal = heatmaps[0][row][col][keypoint]
            maxRow = row
            maxCol = col
          }
        }
      }
      keypointPositions[keypoint] = Pair(maxRow, maxCol)
    }

    // Calculating the x and y coordinates of the keypoints with offset adjustment.
    val xCoords = IntArray(numKeypoints)
    val yCoords = IntArray(numKeypoints)
    val confidenceScores = FloatArray(numKeypoints)
    keypointPositions.forEachIndexed { idx, position ->
      val positionY = keypointPositions[idx].first
      val positionX = keypointPositions[idx].second

      yCoords[idx] = (position.first / (height - 1).toFloat() * bitmap.height + offsets[0][positionY][positionX][idx]).toInt()

      xCoords[idx] = (position.second / (width - 1).toFloat() * bitmap.width + offsets[0][positionY][positionX][idx + numKeypoints]).toInt()

      confidenceScores[idx] = sigmoid(heatmaps[0][positionY][positionX][idx])
    }

    val person = Person()
    val keypointList = Array(numKeypoints) { KeyPoint() }
    var totalScore = 0.0f

    enumValues<BodyPart>().forEachIndexed { idx, it ->
      keypointList[idx].bodyPart = it
      keypointList[idx].position.x = xCoords[idx].toFloat();
      keypointList[idx].position.y = yCoords[idx].toFloat();
      keypointList[idx].score = confidenceScores[idx]
      totalScore += confidenceScores[idx]
    }

    person.keyPoints = keypointList.toList()
    person.score = totalScore / numKeypoints

    return person
  }*/
};



