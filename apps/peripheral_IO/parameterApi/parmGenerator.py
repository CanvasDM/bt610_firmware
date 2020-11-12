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
        self.paramListTotal = 0
        self.resultList = 0
        self.resultSizeList = 0
        self.AttributeTotal = 0
        self.headerFilePath = "../include/"
        self.sourceFilePath = "../src/"
        self.fileName = "AttributeFunctions"
        self.minName = 'minimum'
        self.maxName = 'maximum'

        self.inputHeaderFileName = ""
        self.outputHeaderFileName = ""
        self.inputSourceFileName = ""
        self.outputSourceFileName = ""
        self.jsonFileName = "" 
        self.xlsx_in = ""
        self.xlsx_tab = ""
        self.functionCategory =[]
        self.functionNames = []
        self.functionId = []      
        self.AttributeMax = []
        self.AttributeMin = []
        self.AttributeName = []
        self.AttributeType = []
        self.AttributeBackup = []
        self.AttributeLockable = []
        self.AttributeBroadcast = []
        self.AttributeStringMax = []
        self.AttributeDefault = []
        self.resultName = []
        self._LoadConfig(fname)

    def _LoadConfig(self, fname: str) -> None:
        with open(fname, 'r') as f:
            data = json.load(f)
            self.methodList = data['methods']
            self.toatalFunctions = len(self.methodList)
            file_name = self.headerFilePath +self.fileName
            self.inputHeaderFileName = file_name
            self.outputHeaderFileName = file_name + ""
            file_name = self.sourceFilePath +self.fileName
            self.inputSourceFileName = file_name
            self.outputSourceFileName = file_name + ""


            for i in range(self.toatalFunctions):
                self.functionNames.append(self.methodList[i]['name'])
                self.functionId.append(self.methodList[i]['x-id'])

                #Parameter Data
                self.paramList = self.methodList[i]['params']
                self.paramSizeList = len(self.paramList)
                #Result Data
                self.resultList = self.methodList[i]['result']['schema']['items']
                self.resultSizeList = len(self.resultList)
                if self.paramSizeList > 0:
                    #Read Write because it is a set functions                    
                    self.AttributeTotal = self.AttributeTotal + self.paramSizeList
                    for j in range(self.paramSizeList):
                        self.functionCategory.append("rw")
                        self.AttributeName.append(self.paramList[j]['name'])
                        self.AttributeMax.append(self.paramList[j]['schema'][self.maxName])
                        self.AttributeMin.append(self.paramList[j]['schema'][self.minName])
                        self.AttributeDefault.append(self.paramList[j]['schema']['x-default'])
                        self.AttributeType.append(self.paramList[j]['schema']['x-ctype'])                        
                        self.AttributeStringMax.append(self.paramList[j]['schema']['maximumlength'])
                        self.AttributeBackup.append(self.paramList[j]['schema']['x-backup'])
                        self.AttributeLockable.append(self.paramList[j]['schema']['x-lockable'])
                        self.AttributeBroadcast.append(self.paramList[j]['schema']['x-broadcast'])
                else:
                    #Read only because it is a get function 
                    self.AttributeTotal = self.AttributeTotal + self.resultSizeList
                    for k in range(self.resultSizeList):
                        self.functionCategory.append("ro")
                        if "_duplicate" in self.resultList[k]['name']:
                            # don't add already placed in code as Read/Write
                            self.AttributeTotal = self.AttributeTotal - 1
                        else:    
                            self.AttributeName.append(self.resultList[k]['name'])       
                            self.AttributeType.append(self.resultList[k]['x-ctype']) 
                            self.AttributeStringMax.append(self.resultList[k]['maximumlength'])
                            self.AttributeDefault.append(self.resultList[k]['x-default'])
                            self.AttributeMax.append(self.resultList[k]['maximum'])
                            self.AttributeMin.append(self.resultList[k]['minimum'])
                            self.AttributeBackup.append(self.resultList[k]['x-backup'])
                            self.AttributeLockable.append(self.resultList[k]['x-lockable'])
                            self.AttributeBroadcast.append(self.resultList[k]['x-broadcast'])
            pass    

    def _GetAttributeMacro(self, itype: str, category: str, name: str) -> str:
        """Get the c-macro for the RW or RO attribute"""
        # the order is important here because all protocol values are read-only
        if category == "protocol":
            if itype == "char":
                return "RP_ATTRS(" + name + ")"
            else: 
                return "RP_ATTRX(" + name + ")"
        elif itype == "char":
            if category == "ro":
                return "RO_ATTRS(" + name + ")"
            else: # rw
                return "RW_ATTRS(" + name + ")"
        elif category == "ro":
            return "RO_ATTRX(" + name + ")"
        else:
            return "RW_ATTRX(" + name + ")"

    def _GetValidatorString(self, i_type: str) -> str:
        if i_type == "char":
            return "AttributeValidator_" + "GenericString"
        else:
            return "AttributeValidator_" + (i_type)

    def _CreateMinMaxString(self, imin: str, imax: str, i_type: str) -> str:
        """Create the min/max portion of the attribute table entry"""
        if i_type == "char":
            # string validation is different and doesn't use min/max
            return "0, 0"
        elif i_type == "float":
            return "(uint32_t)" + str(imin) + ", " + "(uint32_t)" + str(imax)
        else:
            if int(imin) < 0:
                s_min = "(uint8_t)" + str(imin)
            else:
                s_min = str(imin)
            if int(imax) < 0:
                s_max = "(uint32_t)" + str(imax)
            else:
                s_max = str(imax)
            return s_min + ", " + s_max

    def _CreateAttrTable(self) -> str:
        """
        Create the attribute (property) table from the dictionary of lists 
        created from the Excel spreadsheet and gperf
        """
        attributeTable = []
        for i in range(0, self.AttributeTotal):
            category = self.functionCategory[i]
            name = self.AttributeName[i]
            i_type = self.AttributeType[i]
            i_min = self.AttributeMin[i]
            i_max = self.AttributeMax[i]
            backup = self.AttributeBackup[i]
            lockable = self.AttributeLockable[i]
            broadcast = self.AttributeBroadcast[i]
            #validator = self.props["Validator"][i].strip()
            number = i
            result = f"  [{number:<2}] = " \
                    + "{ " \
                    + f"{self._GetAttributeMacro(i_type, category, name):<48}, {i_type}, {backup}, {lockable}, {broadcast}, {self._GetValidatorString(i_type):<33}, {self._CreateMinMaxString(i_min, i_max, i_type)}" \
                    + " }," \
                    + "\n"
            attributeTable.append(result)

        attributeTable.append("\n")
        
        #for unused in self.unusedHashes:
        #    result = f"  [{unused:<2}] = " \
        #            + "{ATTR_UNUSED},\n"
        #    attributeTable.append(result)

        string = ''.join(attributeTable)
        return string[:string.rfind(',')]  + '\n'

    def _GetStringSize(self, itype: str, imax: str) -> str:
        if itype == "char":
            return f"[{imax}+1]" # add one for the NUL character
        else:
            return ""

    def _GetDefault(self, itype: str, default: str) -> str:
        if default == "NA":
            if itype == "char":
                return '""'
            elif itype == "float":
                return 0.0
            else:
                return 0
        else:
            if itype == "char":
                return ('"' + default + '"')
            else:
                return default            

    def UpdateFiles(self) -> None:
        """Update the attribute c/h files.  
        minHashStringLength is specific to each hash to prevent out-of-bounds array index.
        The suffix can be used to prevent the input files from being overwritten"""
        #self._CheckLists()
        self._CreateAttributeSourceFile(self._CreateInsertionList(self.inputSourceFileName + ".c"))
        self._CreateAttributeHeaderFile(self._CreateInsertionList(self.inputHeaderFileName + ".h"))

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

    def _CreateStruct(self, category: str, default_values: bool, remove_last_comma: bool) -> str:
        """Creates the structures and default values for RW and RO attributes"""
        struct = []
        for i in range(0, self.AttributeTotal):
            if category == self.functionCategory[i]:
                name = self.AttributeName[i]
                i_type = self.AttributeType[i]
                i_max = self.AttributeStringMax[i]
                default = self.AttributeDefault[i]
                if default_values:
                    result = f"  .{name} = {self._GetDefault(i_type, default)}," + "\n"
                else:
                    result = f"  {i_type} {name}{self._GetStringSize(i_type, i_max)};" + "\n"
                struct.append(result)

        string = ''.join(struct)
        if default_values and remove_last_comma:
            return string[:string.rfind(',')] + '\n'
        else:
            return string


    def _CreateAttributeSourceFile(self, lst: list) -> None:
        """Create the settings/attributes/properties *.c file"""
        name = self.outputSourceFileName + ".c"
        print("Writing " + name)
        with open(name, 'w') as fout:
            for index, line in enumerate(lst):
                if "pystart - " in line:
                    if "attribute table" in line:
                        lst.insert(index + 1, self._CreateAttrTable())
                    #elif "asso values" in line:
                    #    lst.insert(index + 1, self._GetAssoTable())
                    #elif "hash function" in line:
                    #    lst.insert(index + 1, self._GetHashBlock())
                    elif "rw attributes" in line:
                        lst.insert(index + 1, self._CreateStruct("rw", False, False))
                    elif "rw defaults" in line:
                        lst.insert(index + 1, self._CreateStruct("rw", True, True))
                    elif "ro attributes" in line:
                        lst.insert(index + 1, self._CreateStruct("ro", False, True))
                    elif "ro defaults" in line:
                        lst.insert(index + 1, self._CreateStruct("ro", True, True))
                    #elif "hash info" in line:
                    #    lst.insert(index + 1, self._GetHashInfo())
            fout.writelines(lst)        

    def _CreateAttrIndices(self) -> str:
        """Create attribute indices for header file"""
        indices = []
        for i in range(0, self.AttributeTotal):
            name = self.AttributeName[i]
            id = i
            result = f"#define ATTR_INDEX_{name:<37} {id}" + "\n"
            indices.append(result)
        return ''.join(indices)

    def _CreateAttrDefinitions(self) -> str:
        """Create some definitinons for header file"""
        defs = []
        defs.append(f"#define ATTRIBUTE_FUNCTION_TABLE_SIZE {self.AttributeTotal}\n\n")
        #defs.append(f"#define ATTRIBUTE_TOTAL_KEYWORDS {self.totalKeywords}\n")
        return ''.join(defs)

    def _CreateAttributeHeaderFile(self, lst: list) -> None:
        """Create the attribute header file"""
        name = self.outputHeaderFileName + ".h"
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
    
    
