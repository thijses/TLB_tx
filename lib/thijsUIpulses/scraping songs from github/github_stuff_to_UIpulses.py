## some code for turning Arduino C code from a github repo into code that my own library will accept
## (opens all the .ino files in the vicinity and looks for the first array it can find)
## (note: does not handle duplicate input .ino filenames)


## first, find and open some files
def getFileList():
    import codeCounter
    codeCounter.fileExtensions = ['.ino']
    codeCounter.findFiles(codeCounter.os.path.split(__file__)[0])
    # print("found .ino files:", len(codeCounter.fileList))
    returnList = dict([(codeCounter.os.path.splitext(codeCounter.os.path.split(pathAsIndex)[1])[0], [codeCounter.fileList[pathAsIndex]]) for pathAsIndex in codeCounter.fileList])
    return(returnList) # should return a dict where the keys are the tune names and the contents are the file handler (text file objects)

filesToProcess = getFileList()
# print("found:"); [print(tuneName) for tuneName in filesToProcess]
# [print("playTune("+tuneName+');  Serial.println("'+tuneName+'"); buzzerHandler.finishPulses();') for tuneName in filesToProcess]
# print("UIpulse* tuneList[] = {"); [print("tunes::"+tuneName+",") for tuneName in filesToProcess]; print("};")
# print("String tuneNameList[] = {"); [print('"'+tuneName+'",') for tuneName in filesToProcess]; print("};")

## now attempt to retrieve the note array from the files
for tuneName in filesToProcess:
    # print("interpreting:", tuneName)
    file = filesToProcess[tuneName][0]
    inputArrayString = ""
    arrayStartFound = False
    ignoreLongComment = False # a quick hack to fix /* */ support
    line: str
    for line in file:
        if(not arrayStartFound):
            arrayStartStr = " = {"
            arrayStartIndex = line.find(arrayStartStr)
            if(arrayStartIndex >= 0): # a somewhat crude test to find the start of the 
                arrayStartFound = True
                line = line[(arrayStartIndex+len(arrayStartStr)):] # cut off first part
        if(arrayStartFound):
            ## first, a quick long-comment /* */ workaround:
            if(ignoreLongComment): # if a long comment is ongoing for multiple lines
                longCommentEndIndex = line.find("*/")
                if(longCommentEndIndex >= 0):
                    line = line[(longCommentEndIndex+2):] # include everything after the */
                    ignoreLongComment = False
            if(ignoreLongComment): # if ignoreLongComment is still true
                continue # skip this line entirely
            while(line.find("/*") >= 0): # you may need to remove several bounded long-comments
                longCommentStartIndex = line.find("/*")
                if(longCommentStartIndex >= 0):
                    ignoreLongComment = True
                    longCommentEndIndex = line.find("*/")
                    if(longCommentEndIndex >= 0):
                        line = line[:longCommentStartIndex] + line[(longCommentEndIndex+2):] # carve out the bounded comment
                        ignoreLongComment = False
                    else: # otherwise, the long-comment extends to the next line
                        line = line[:longCommentStartIndex]
            ## first, strip comments:
            commentIndex = line.find("//") # assumes there are only 2 slashes ()
            if(commentIndex >= 0):
                line = line[:commentIndex]
            ## next, see if this line marks the end of the array
            arrayEndFound = False
            if(line.rfind(';') >= 0):
                lastBracketIndex = line.rfind('}') # if the last bracket is in this file
                if(lastBracketIndex >= 0): # if the last bracket is in this line
                    line = line[:lastBracketIndex] # cutt off any crap past the last bracket
                else: # if the last bracket is NOT in the current like (unlikely)
                    print("file:",tuneName," removing last bracket from string:", line, lastBracketIndex, inputArrayString[max(-10, -len(inputArrayString)):], max(-10, -len(inputArrayString)))
                    lastBracketIndex = inputArrayString.rfind('}') # if the last bracket is in this file
                    if(lastBracketIndex >= 0): # if the last bracket is in the arrayString
                        inputArrayString = inputArrayString[:lastBracketIndex]
                    else: # panic
                        print("couldn't find last bracket anywhere! aborting")
                        # print(inputArrayString)
                        break
                arrayEndFound = True
            ## first, strip comments:
            line = line.strip("\n").strip()
            inputArrayString += line
            if(arrayEndFound):
                break
    filesToProcess[tuneName].append(inputArrayString) # save the array string

## this one i just scraped manually (with the help of some clever CTRL+F work)
toneMap = {
"NOTE_B0" : 31,
"NOTE_C1" : 33,"NOTE_D1" : 37,"NOTE_DS1" : 39,"NOTE_E1" : 41,"NOTE_F1" : 44,"NOTE_FS1" : 46,"NOTE_G1" : 49,"NOTE_GS1" : 52,"NOTE_A1" : 55,"NOTE_AS1" : 58,"NOTE_B1" : 62,
"NOTE_C2" : 65,"NOTE_CS2" : 69,"NOTE_D2" : 73,"NOTE_DS2" : 78,"NOTE_E2" : 82,"NOTE_F2" : 87,"NOTE_FS2" : 93,"NOTE_G2" : 98,"NOTE_GS2" : 104,"NOTE_A2" : 110,"NOTE_AS2" : 117,"NOTE_B2" : 123,
"NOTE_C3" : 131,"NOTE_CS3" : 139,"NOTE_D3" : 147,"NOTE_DS3" : 156,"NOTE_E3" : 165,"NOTE_F3" : 175,"NOTE_FS3" : 185,"NOTE_G3" : 196,"NOTE_GS3" : 208,"NOTE_A3" : 220,"NOTE_AS3" : 233,"NOTE_B3" : 247,
"NOTE_C4" : 262,"NOTE_CS4" : 277,"NOTE_D4" : 294,"NOTE_DS4" : 311,"NOTE_E4" : 330,"NOTE_F4" : 349,"NOTE_FS4" : 370,"NOTE_G4" : 392,"NOTE_GS4" : 415,"NOTE_A4" : 440,"NOTE_AS4" : 466,"NOTE_B4" : 494,
"NOTE_C5" : 523,"NOTE_CS5" : 554,"NOTE_D5" : 587,"NOTE_DS5" : 622,"NOTE_E5" : 659,"NOTE_F5" : 698,"NOTE_FS5" : 740,"NOTE_G5" : 784,"NOTE_GS5" : 831,"NOTE_A5" : 880,"NOTE_AS5" : 932,"NOTE_B5" : 988,
"NOTE_C6" : 1047,"NOTE_CS6" : 1109,"NOTE_D6" : 1175,"NOTE_DS6" : 1245,"NOTE_E6" : 1319,"NOTE_F6" : 1397,"NOTE_FS6" : 1480,"NOTE_G6" : 1568,"NOTE_GS6" : 1661,"NOTE_A6" : 1760,"NOTE_AS6" : 1865,"NOTE_B6" : 1976,
"NOTE_C7" : 2093,"NOTE_CS7" : 2217,"NOTE_D7" : 2349,"NOTE_DS7" : 2489,"NOTE_E7" : 2637,"NOTE_F7" : 2794,"NOTE_FS7" : 2960,"NOTE_G7" : 3136,"NOTE_GS7" : 3322,"NOTE_A7" : 3520,"NOTE_AS7" : 3729,"NOTE_B7" : 3951,
"NOTE_C8" : 4186,"NOTE_CS8" : 4435,"NOTE_D8" : 4699,"NOTE_DS8" : 4978,
"REST" : 0 }


## now convert the input array strings into duration, duty-cycle and frequency
for tuneName in filesToProcess:
    # print("converting:", tuneName)
    inputArrayString: str = filesToProcess[tuneName][1]
    inputArray = inputArrayString.split(",") # split into python array (of strings)
    inputArray = [[inputArray[i*2].strip(), inputArray[i*2+1].strip()] for i in range(len(inputArray)//2)] # combine the tones with their durations (as strings)
    inputArray = [[toneMap[tone],int(length)] for tone,length in inputArray] # turn the strings into integers

    def calcDuration(inVal):
        duration = (60000*4/200)/inVal # intiger math
        if(duration < 0):
            duration = abs(duration * 1.5)
        return(int(duration))

    outputArray = []
    for entry in inputArray:
        freq = entry[0]
        if(freq != 0):
            duration = calcDuration(entry[1])
            duty = 0.9
            outputArray.append([duration, duty, freq])
        else:
            restDuration = calcDuration(entry[1])
            if(len(outputArray) == 0): # if the tune starts with a REST (for some weird reason)
                # print("doing crappy REST-as-start workaround!", tuneName) # a little debug
                # outputArray.append([restDuration, 0.01, 1]) # freq = 0, duty cycle = 0, my code is not gonna love this.
                print("skipping REST as start in", tuneName)
            else: # normal case:
                duration = outputArray[-1][0];  duty = outputArray[-1][1] # read
                duty = duty * (duration / (duration + restDuration)) # adjust duty cycle to include the extra duration
                duration += restDuration
                outputArray[-1][0] = duration;  outputArray[-1][1] = duty # write
    filesToProcess[tuneName].append(outputArray) # save code output array

## then into strings of code in my own custom format
makeDefineName = lambda tuneName : "BUZ_TUNE_"+tuneName.upper() # one (consistant) function for generating a suitable #define name 
for tuneName in filesToProcess:
    outputArray = filesToProcess[tuneName][2]
    ## calculate some fun stats:
    tuneDuration=0;  tuneArraySize = 0 # some fun counters for the length of the tune and how much space it will occupy on the microcontroller
    for duration,_,_ in outputArray:
        tuneDuration += duration
        tuneArraySize += (4+4+4+4) # UIpulse objects are 16 bytes large
    statCommentStr = "// duration: "+str(round(tuneDuration/1000.0, 1))+" secs,  size:"+str(tuneArraySize)+" bytes,  efficiency: "+str(round(tuneDuration/tuneArraySize, 2))
    filesToProcess[tuneName].append([tuneDuration, tuneArraySize, statCommentStr]) # save the stats
    ## generate custom code:
    #PWMpowerStr = "tuneVolumeDuty" # the name of a macro the user should define in the code
    PWMpowerStr = makeDefineName(tuneName) # use the tune specific #define name for the volume
    codeString = "UIpulse " + tuneName + "["+str(len(outputArray))+"] = { "+statCommentStr+"\n"
    for entry in outputArray:
        codeLine = "    {"
        codeLine += str(int(entry[0])) + ", "
        codeLine += str(round(entry[1], 3)) + ", "
        codeLine += str(int(entry[2])) + ", " # does not need to be an integer
        codeLine += PWMpowerStr
        codeLine += "},\n"
        codeString += codeLine
    codeString += "};"
    filesToProcess[tuneName].append(codeString) # save code output string

## some fun debug:
maxTuneNameLength = 0
for tuneName in filesToProcess:
    maxTuneNameLength = max(maxTuneNameLength, len(tuneName))
totalDuration=0;  totalArraySize = 0 # some fun counters for the sum of ALL tunes
for tuneName in filesToProcess:
    tuneDuration, tuneArraySize, _ = filesToProcess[tuneName][3]
    totalDuration += tuneDuration;  totalArraySize += tuneArraySize
    # print(tuneName, (maxTuneNameLength - len(tuneName))*" "+"  tune duration:\t", round(tuneDuration/1000.0, 1), "seconds,   size:", tuneArraySize, "bytes   efficiency:", round(tuneDuration/tuneArraySize, 2))
print("all durations summed:", round(totalDuration/1000.0, 1), "seconds, size:", totalArraySize, "bytes ==", round(totalArraySize/1000, 3), "KB")
sortedByDuration = sorted(filesToProcess, key=lambda tuneName : filesToProcess[tuneName][3][0], reverse= False)
print("shortest tune:", sortedByDuration[0], round(filesToProcess[sortedByDuration[0]][3][0]/1000.0, 1), "seconds")
print("longest  tune:", sortedByDuration[-1], round(filesToProcess[sortedByDuration[-1]][3][0]/1000.0, 1), "seconds")
sortedBySize = sorted(filesToProcess, key=lambda tuneName : filesToProcess[tuneName][3][1], reverse= False)
print("smallest size:", sortedBySize[0], filesToProcess[sortedBySize[0]][3][1], "bytes")
print("biggest  size:", sortedBySize[-1], filesToProcess[sortedBySize[-1]][3][1], "bytes")

## finally, create one huge .h file with all the tunes, as well as #define statements for which ones the user wants to include in compilation
defaultVolumeDuty = 0.05 # duty cycle of PWM signal, determines buzzer volume. 0.5 should be the max (i think)
outputFile = open("buzzer_tunes.h", "w")
outputFile.write('\n\
/*\n\
this tab contains some fun sounds\n\
to use them, use #define for each tune you want to compile,\n\
make sure to add the PWM duty cycle (volume) as the defined value!\n\
the maximum volume should be at 0.5. i found 0.1 to be a lot more pleasant in most cases.\n\
(see examples below)\n\
then #include this file\n\
*/\n\
\n\
// #pragma once  // just use the ifndef method\n\
#ifndef BUZZER_TUNES_H\n\
#define BUZZER_TUNES_H\n\
\n\
#include "thijsUIpulses.h" // the library which holds the UIpulse struct\n\
\n\
//#define tuneVolumeDuty  '+str(defaultVolumeDuty)+' // (switched to individual volumes)\n\
\n')

for tuneName in filesToProcess:
    statCommentStr = filesToProcess[tuneName][3][2]
    outputFile.write("//#define "+makeDefineName(tuneName)+"  "+str(defaultVolumeDuty)+"  "+((maxTuneNameLength - len(tuneName))*" ")+statCommentStr+"\n")
outputFile.write('\n\
namespace tunes { // tunes are contained in this namespace to avoid name collisions\n\
\n')
for tuneName in filesToProcess:
    defineName = makeDefineName(tuneName)
    outputFile.write("#ifdef "+defineName+"\n")
    outputFile.write("  "+filesToProcess[tuneName][-1]+"\n") # write the big code sequence
    outputFile.write("#endif // "+defineName+"\n\n")
## write the last bits
outputFile.write('\n\
};\n\
\n\
#endif // BUZZER_TUNES_H\n\
\n')
outputFile.close()