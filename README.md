# BuildAndRun
A program that builds and runs all of your other programs.

If you know multiple programming languages, it can kind of be a pain to remember all of there separate commands, muchless not mix them up. With this program, you write the command to compile and execute a particular language once, and then you don't have to worry about it again.

### To Download
Follow this [link](https://github.com/carmichaeljr/BuildAndRun/releases/tag/v1.0) to download the program. Note that this program has only been tested on windows and it will most likely not work on other platforms.

### To Use
##### Settings And Setup
The very first thing that you have to do to use this program is update the settings file (A xml file). To do this, navigate to the folder where the application is, and run the program with the 's' flag as the first argument. Examples are shown below.
* On Linux: ```./BuildAndRun s```
* On Windows: ```BuildAndRun.exe s```

This will open the settings file for you to edit. From here you can edit the *LanguageSettings* part of the settings file. For each language, there are three things that you need to worry about.
* This first thing is the language definition. With this you need to define the extension that is identified with the language, and set the output path if that languages compiler needs one.
  * ```<LangDef ext=c outputPath=""></LangDef>```
* After that you need to set the compiler command. This will need the path to the compiler, and all of its arguments. Each argument has one flag, *addSpace*. If this is true then there will be a space added at the end of that argument. If false no space is added. This is usefull for combining multiple argument tags into one. An example of a complete compile command is shown below. You may have noticed the ```<MainFile>``` tag. This is a special that that inserts the main file that you give the program. There are three special tags. They are listed below with how to use them.
  ```
  <CompileCmd cmd="C:\MinGW\bin\gcc.exe">
    <Arg addSpace=true>-Wall -Wpointer-arith -g -o</Arg>
    <MainFile name=true extension=false></MainFile>
    <Arg addSpace=true>*.h</Arg>
    <Arg addSpace=true>*.c</Arg>
  </CompileCmd>
  ```
   * ```<MainFile name=true extension=false></MainFile>```: This inserts the main file name, extension, or both depending on the flags given
   * ```<OutputPath></OutputPath>```: This inserts the output path that you defined with the language. No flags are needed.
   * ```<CurrentDirectory></CurrentDirectory>```: This inserts the current directory that the program is run from. No flags are needed.
 * After the compile command you set the execution command in a similar manner.
  
##### Usage
* Navigate to the folder that contains the project you want to compile/execute.
* Run the program with arguments as shown below: 
  ```
  On Linux: path/to/buildAndRunFolder/BuildAndRun [option] [file]
  On Windows: C:\path\to\buildAndRunFolder\BuildAndRun.exe [option] [file]
  ```
    * The *Option* argument can be: s,d,c,e,l,ce,cl,el,cel
      * 's': open settings for editing. This command expects no second argument.
      * 'd': run debug. This command expects no second argument.
      * 'c': compile the program given by the second argument.
      * 'e': execute the program given by the second argument.
      * 'l': run line count on the program given by the second argument.
      * ...and the above shown combinations.
    * The *File* argument is the name of the main file in the project
* A xml file with the same name as the main file will be created in the curernt folder. The contents of this file will be based upon the extension of the file you gave as the second argument, and the corresponding language definition in the settings file. This is the projects *build file*.
  * Any subsiquent calls to the program will use this build file, and not the settings file as reference.
  * To add files to the line count operation, just create another ```<Files>``` tag with the desired folder and wildcard expression.
