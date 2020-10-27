For all those of you needing an easy-access API for the using the Tensorflow Posenet posenet_model.tflite file in C++ code, here it is. It's modeled on Tensorflow's example Java file linked in the description.

NOTE: I'll be running speed tests comparing the use of 4D C++ std::vector<>'s versus regular 4D C arrays (float\*\*\*\*). It seems as though the speeds should be comparable, but I'm guessing primitive C arrays are the way to go.
