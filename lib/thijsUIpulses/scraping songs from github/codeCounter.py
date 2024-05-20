# todo: include extended comment sections like /* */ in C and """ """ in python

import os # for file handling
import sys # for cmdline arguments
import glob # for searching folders?
# print(os.getcwd())
# print(sys.argv)

fileExtensions = {
    '.py' : 0,
    '.ino' : 0,
    '.h' : 0,
    '.cpp' : 0
}
commentSymbols = {
    '.py' : '#',
    '.ino' : '//',
    '.h' : '//',
    '.cpp' : '//',
    '.c' : '//',
}
# longCommentSymbols = {
#     '.py' : ('"""', '"""'),
#     '.ino' : ('/*', '*/'),
#     '.h' : ('/*', '*/'),
#     '.cpp' : ('/*', '*/'),
#     '.c' : ('/*', '*/'),
# }
fileList = {}
maxDepth = 3

def findFiles(path, depth=0):
    path = path.strip('"') # if the argument is a folder, it may end with '\"", which gets interpreted as '"' (becuase backslash is used to denote the use of special characters)
    # print("\n path:", path)
    # print("depth:", depth)
    _, extension = os.path.splitext(path)
    if(extension == ''): # it's a folder
    #if(os.path.isdir(path)): # it's a folder
        if(depth < maxDepth):
            #print("searching folder:", path)
            globSearchPath = os.path.join(os.path.realpath(path), '*')
            for subPath in glob.glob(globSearchPath):
                findFiles(subPath, depth+1)
        # else:
        #     print("not searching folder:",path,", max depth reached!")
    elif((extension in fileExtensions) or (depth == 0)):
        #print("attempting to add file:", path)
        try:
            fileToInspect = open(path, "rt")
            fileList[path] = fileToInspect # use the path as the key for the fileList dict (using only the filename would overwrite identically named files)
        except Exception as excep:
            print("exception:", excep, " when trying to open file:", path,)

def scanFile(path, file):
    codeLineCount = 0
    commentLineCount = 0 # lines of JUST comment
    commentedCodeCount = 0 # code lines with comments at the end
    _, extension = os.path.splitext(path)
    extension = extension.lower()
    commentSymbol = (commentSymbols[extension] if (extension in commentSymbols) else None)
    if(commentSymbol is None):
        print("could not identify comment sybol for:", os.path.split(path)[1])
    for line in file:
        line = line.lstrip(' \t')
        if(len(line) <= 1): # don't count empty lines
            continue
        elif(line.startswith(commentSymbol) if (commentSymbol is not None) else False):
            commentLineCount += 1
        else:
            codeLineCount += 1
            if((commentSymbol in line) if (commentSymbol is not None) else False):
                commentedCodeCount += 1
    return(codeLineCount, commentLineCount, commentedCodeCount)

if(__name__ == "__main__"):
    pathList = [item for item in sys.argv[1:]]
    if(len(pathList) == 0):
        print("(you're supposed to pass files as cmdline arguments or dragged onto windows shortcuts)")
        inputPath = input("please enter a file path here: ")
        if(len(inputPath) > 0):
            pathList.append(inputPath)
        else:
            input("no path entered, press enter to exit")
    if(len(pathList) > 0):
        while(True):
            for path in pathList:
                findFiles(path)
            codeLineCountTotal = 0
            commentLineCountTotal = 0
            commentedCodeCountTotal = 0
            for pathAsIndex in fileList:
                try:
                    codeLineCount, commentLineCount, commentedCodeCount = scanFile(pathAsIndex, fileList[pathAsIndex])
                    print("file:", os.path.split(pathAsIndex)[1], "  codeLineCount:", codeLineCount, "  commentLineCount:", commentLineCount, "  commentedCodeCount:", commentedCodeCount)
                    codeLineCountTotal += codeLineCount;   commentLineCountTotal += commentLineCount;   commentedCodeCountTotal += commentedCodeCount
                except Exception as excep:
                    print("failed to scan file:", os.path.split(pathAsIndex)[1],"  exception:", excep)
            if(len(fileList) > 1):
                print("totals:  codeLineCount:", codeLineCountTotal, "  commentLineCount:", commentLineCountTotal, "  commentedCodeCount:", commentedCodeCountTotal, "   at maxDepth:", maxDepth)
            elif(len(fileList) == 0):
                print("no suitable files found")
            # todo maybe: stop loop here if you only entered files, not folders as sys.argv
            newMaxDepth = input("enter a new maxDepth to run again, or enter nothing to exit: ")
            if(len(newMaxDepth) > 0):
                try:
                    maxDepth = int(newMaxDepth)
                    if(maxDepth > 0):
                        fileList = {}
                    else:
                        break
                except:
                    break
            else:
                break