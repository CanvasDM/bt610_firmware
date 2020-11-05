import json
import openpyxl
import collections
import os
import getpass



class attributes:
    def __init__(self, fname: str = "BT6Api.json"):

        # The following items are loaded from the configuration file
        self.methodList = 0
        self.toatalFunctions = 0
        self.paramList = 0
        self.paramSizeList = 0
        self.headerFilePath = "../include/"
        self.sourceFilePath = "../src/"
        self.fileName = "AttributeFunctions"
        self.minName = 'minimum'
        self.maxName = 'maximum'

        self.inputFileName = ""
        self.outputFileName = ""
        self.jsonFileName = "" 
        self.xlsx_in = ""
        self.xlsx_tab = ""
        self.columnNames = []
        self.columnId= []
        self._LoadConfig(fname)

    def _LoadConfig(self, fname: str) -> None:
        with open(fname, 'r') as f:
            data = json.load(f)
            self.methodList = data['methods']
            self.toatalFunctions = len(self.methodList)
            file_name = self.headerFilePath +self.fileName
            
            self.inputFileName = file_name
            self.outputFileName = file_name + ""

            for i in range(self.toatalFunctions):
                #print(methodList[i]['name'])
                #print(methodList[i]['x-id'])
                self.columnNames.append(self.methodList[i]['name'])
                self.columnId.append(self.methodList[i]['x-id'])
                #Parameter Data
                self.paramList = self.methodList[i]['params']
                self.paramSizeList = len(self.paramList)
                if self.paramSizeList > 0:
                    for j in range(self.paramSizeList):
                        print(self.paramList[j]['schema'][self.minName])
                        #print(self.paramList[j]['schema'][self.maxName])
            pass    
    def UpdateFiles(self) -> None:
        """Update the attribute c/h files.  
        minHashStringLength is specific to each hash to prevent out-of-bounds array index.
        The suffix can be used to prevent the input files from being overwritten"""
        #self._CheckLists()
        #self._CreateAttributeSourceFile(self._CreateInsertionList(self.inputFileName + ".c"))
        self._CreateAttributeHeaderFile(self._CreateInsertionList(self.inputFileName + ".h"))

    def _CreateInsertionList(self, name: str) -> list:
        """Read in the c/h file and create a list of strings that 
        is ready for the attribute hash information to be inserted"""
        print("Reading " + name)
        lst = []
        with open(name, 'r') as fin:
            copying = True
            for line in fin:
                if "pystart" in line:
                    lst.append(line)
                    copying = False
                elif "pyend" in line:
                    lst.append(line)
                    copying = True
                elif copying:
                    lst.append(line)

        return lst

    def _CreateAttrIndices(self) -> str:
        """Create attribute indices for header file"""
        indices = []
        for i in range(0, self.toatalFunctions):
            name = self.columnNames[i]
            id = self.columnId[i]
            result = f"#define ATTR_INDEX_{name:<37} {id}" + "\n"
            indices.append(result)
        return ''.join(indices)

    def _CreateAttrDefinitions(self) -> str:
        """Create some definitinons for header file"""
        defs = []
        defs.append(f"#define ATTRIBUTE_FUNCTION_TABLE_SIZE {self.toatalFunctions}\n\n")
        #defs.append(f"#define ATTRIBUTE_TOTAL_KEYWORDS {self.totalKeywords}\n")
        return ''.join(defs)

    def _CreateAttributeHeaderFile(self, lst: list) -> None:
        """Create the attribute header file"""
        name = self.outputFileName + ".h"
        print("Writing " + name)
        with open(name, 'w') as fout:
            for index,line in enumerate(lst):
                if "pystart - " in line:
                    if "attribute function indices" in line:
                        lst.insert(index + 1, self._CreateAttrIndices())
                    elif "attribute function definitions" in line:
                        lst.insert(index + 1, self._CreateAttrDefinitions())

            fout.writelines(lst)    
    
if __name__ == "__main__":
    a = attributes()
    #a.ImportWorkbook()
    #a.UpdateHash()
    a.UpdateFiles()
    #a.SaveAttributesJson()
    # test loading from file
    #b = attributes()
    #b.LoadAttributesJson()
    #if a == b:
    #    print("match")
    #else:
    #    print("boo")
    
    
