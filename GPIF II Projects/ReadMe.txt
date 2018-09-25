Note that GPIF Designer II uses ABSOLUTE file locations

This is good if you would like to put the generated .h file straight into the Eclipse project folder however this makes them not portable.
You will find that ALL of the output files will go to C:\Users\John\Documents\GPIF II Designer\<project folder> which is where I kept them
This is probably NOT where you want them to be, so, once you open an example:
	Go to "State Machine" then, under "Build" change "Build Settings" to point to a directory on your PC