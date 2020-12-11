import json
import openpyxl
import collections
import os
import getpass



class attributes:
    def __init__(self, fname: str = "BT6ApiParams.json"):

        # The following items are loaded from the configuration file
        self.componetList = 0
        self.toatalParameters = 0

        self.headerFilePath = "../../common/include/"
        self.sourceFilePath = "../../common/source/"
        self.fileName = "AttributeFunctions"

        self.inputHeaderFileName = ""
        self.outputHeaderFileName = ""
        self.inputSourceFileName = ""
        self.outputSourceFileName = ""
        self.jsonFileName = "" 

        self.functionCategory =[]
        self.ParamNames = []
        self.ParamId = []      
        self.AttributeMax = []
        self.AttributeMin = []
        self.AttributeName = []
        self.AttributeSummary = []
        self.AttributeType = []
        self.AttributeBackup = []
        self.AttributeLockable = []
        self.AttributeBroadcast = []
        self.AttributeStringMax = []
        self.AttributeDefault = []
        self.AttributeReadOnly = []
        self.AttributeWriteOnly = []
        self.AttributeReadWrite = []
        self.resultName = []
        self._LoadConfig(fname)

    def _LoadConfig(self, fname: str) -> None:
        with open(fname, 'r') as f:
            data = json.load(f)
            self.componetList = data['components']['contentDescriptors']['deviceParams']['x-deviceparameters']
            self.toatalParameters = len(self.componetList)
            file_name = self.headerFilePath +self.fileName
            self.inputHeaderFileName = file_name
            self.outputHeaderFileName = file_name + ""
            file_name = self.sourceFilePath +self.fileName
            self.inputSourceFileName = file_name
            self.outputSourceFileName = file_name + ""


            for i in range(self.toatalParameters):
                self.ParamNames.append(self.componetList[i]['name'])
                self.ParamId.append(self.componetList[i]['x-id'])
                #schema sub
                self.AttributeStringMax.append(self.componetList[i]['schema']['maximumlength'])
                self.AttributeMax.append(self.componetList[i]['schema']['maximum'])
                self.AttributeMin.append(self.componetList[i]['schema']['minimum'])
                self.AttributeDefault.append(self.componetList[i]['schema']['x-default'])
                self.AttributeType.append(self.componetList[i]['schema']['x-ctype'])  
                self.AttributeBackup.append(self.componetList[i]['schema']['x-backup'])
                self.AttributeLockable.append(self.componetList[i]['schema']['x-lockable'])
                self.AttributeBroadcast.append(self.componetList[i]['schema']['x-broadcast'])
                self.AttributeReadOnly.append(self.componetList[i]['schema']['x-readonly'])
                self.AttributeWriteOnly.append(self.componetList[i]['schema']['x-writeonly'])
                self.AttributeReadWrite.append(self.componetList[i]['schema']['x-readwrite'])             
            pass    

    def _GetType(self, itype: str) -> str:
        if itype == "char":
            return "s"
        elif "int" in itype:
            return "i"
        elif itype == "float":
            return "f"
        else:
            return "u"
    def _GetAttributeMacro(self, itype: str, readWrite: bool, readOnly: bool, name: str) -> str:
        """Get the c-macro for the RW or RO attribute"""
        # the order is important here because all protocol values are read-only
        if itype == "char":
            if readOnly == True:
                return "RO_ATTRS(" + name + ")"
            elif readWrite == True:
                return "RW_ATTRS(" + name + ")"
        elif readOnly == True:
            return "RO_ATTRX(" + name + ")"
        elif readWrite == True:
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
            return "(float)" + str(imin) + ", " + "(float)" + str(imax)
        else:
            if int(imin) < 0:
                s_min = f"({i_type})" + str(imin)
                s_min = s_min[:-2]
            else:
                s_min = str(imin)
                s_min = s_min[:-2]
            if int(imax) < 0:
                s_max = f"({i_type})" + str(imax)
                s_max = s_max[:-2]             
            else:
                s_max = str(imax)
                s_max = s_max[:-2] 
            return s_min + ", " + s_max

    def _CreateAttrTable(self) -> str:
        """
        Create the attribute (property) table from the dictionary of lists 
        created from the Excel spreadsheet and gperf
        """
        attributeTable = []
        for i in range(0, self.toatalParameters):
            readWrite = self.AttributeReadWrite[i]
            readOnly = self.AttributeReadOnly[i]
            name = self.ParamNames[i]
            i_type = self.AttributeType[i]
            i_min = self.AttributeMin[i]
            i_max = self.AttributeMax[i] 
            backup = str(self.AttributeBackup[i])
            lockable = str(self.AttributeLockable[i])
            broadcast = str(self.AttributeBroadcast[i])
            #validator = self.props["Validator"][i].strip()
            i_hash = i
            result = f"  [{i_hash:<2}] = " \
                    + "{ " \
                    + f"{self._GetAttributeMacro(i_type, readWrite, readOnly, name):<48}, {self._GetType(i_type)}, {backup.lower()}, {lockable.lower()}, {broadcast.lower()}, {self._GetValidatorString(i_type):<33}, {self._CreateMinMaxString(i_min, i_max, i_type)}" \
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
        for i in range(0, self.toatalParameters):
            if ((category == 'rw') & (self.AttributeReadWrite[i] == True)) or ((category == 'ro') & (self.AttributeReadOnly[i] == True)):
                name = self.ParamNames[i]
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
        for i in range(0, self.toatalParameters):
            name = self.ParamNames[i].upper()
            #id = self.functionId[i]
            result = f"#define ATTR_INDEX_{name:<37} {i}" + "\n"
            indices.append(result)
        return ''.join(indices)

    def _CreateAttrDefinitions(self) -> str:
        """Create some definitinons for header file"""
        defs = []
        defs.append(f"#define ATTRIBUTE_TABLE_SIZE {self.toatalParameters}\n\n")
        #defs.append(f"#define ATTRIBUTE_TOTAL_KEYWORDS {self.toatalFunctions}\n")
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
    
    
