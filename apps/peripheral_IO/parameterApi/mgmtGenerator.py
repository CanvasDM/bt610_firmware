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
        self.paramListTotal = 0
        self.resultList = 0
        self.resultSizeList = 0
        self.AttributeTotal = 0
        self.headerFilePath = "../include/"
        self.sourceFilePath = "../src/"
        self.fileName = "Sentrius_mgmt"
        self.handlerfileName = "Sentrius_mgmtHandlers"
        self.minName = 'minimum'
        self.maxName = 'maximum'
        self.mgmtIdPrefex = 'SENTRIUS_MGMT_ID_'

        self.inputHeaderFileName = ""
        self.outputHeaderFileName = ""
        self.inputSourceFileName = ""
        self.outputSourceFileName = ""
        self.jsonFileName = "" 
        self.xlsx_in = ""
        self.xlsx_tab = ""
        self.functionCategory =[]
        self.functionNames = []
        self.paramSizeList = []
        self.functionId = []      
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
        self.resultName = []
        self.indices = []
        self.handlerFunctionName = []
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

            file_name = self.sourceFilePath +self.handlerfileName
            self.inputSourceHandlerfileName = file_name
            self.outputSourceHandlerfileName = file_name + ""


            for i in range(self.toatalFunctions):
                self.functionNames.append(self.methodList[i]['name'])
                self.functionId.append(self.methodList[i]['x-id'])

                #Parameter Data
                self.paramList = self.methodList[i]['params']
                self.paramSizeList.append(len(self.paramList))
                #Result Data
                self.resultList = self.methodList[i]['result']['schema']['items']
                self.resultSizeList = len(self.resultList)
                if self.paramSizeList[i] > 0:
                    #Read Write because it is a set functions                    
                    self.AttributeTotal = self.AttributeTotal + self.paramSizeList[i]
                    for j in range(self.paramSizeList[i]):
                        self.functionCategory.append("rw")
                        self.AttributeName.append(self.paramList[j]['name'])
                        self.AttributeSummary.append(self.paramList[j]['summary'])
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
                        if "_duplicate" in self.resultList[k]['summary']:
                            # don't add already placed in code as Read/Write
                            self.AttributeTotal = self.AttributeTotal - 1
                        else:    
                            self.AttributeName.append(self.resultList[k]['name'])  
                            self.AttributeSummary.append(self.resultList[k]['summary'])     
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
            i_hash = i
            result = f"  [{i_hash:<2}] = " \
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
    def _GetCborType(self, itype: str) -> str:
        if itype == "char":
            return "CborAttrTextStringType"
        elif itype == "float":
            return "CborAttrFloatType"
        elif (itype == "uint8_t") or (itype == "uint16_t") or (itype == "uint32_t"):
            return "CborAttrUnsignedIntegerType"
        else:
            return "CborAttrIntegerType"                        

    def UpdateFiles(self) -> None:
        """Update the attribute c/h files.  
        minHashStringLength is specific to each hash to prevent out-of-bounds array index.
        The suffix can be used to prevent the input files from being overwritten"""
        #self._CheckLists()
        self._CreateAttributeHeaderFile(self._CreateInsertionList(self.inputHeaderFileName + ".h"))
        self._CreateAttributeSourceFile(self._CreateInsertionList(self.inputSourceFileName + ".c"))
        self._CreateMgmtHandlerFile(self._CreateInsertionList(self.inputSourceHandlerfileName + ".c"))
        

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
    def _CreateStringDefinitions(self) -> str:
        """Create some definitinons for header file"""
        defs = []
        for i in range(0, self.AttributeTotal):
            name = self.AttributeSummary[i]
            i_type = self.AttributeType[i]
            i_max = self.AttributeStringMax[i]
            if i_type == "char":
                defs.append( f"static char {name}{self._GetStringSize(i_type, i_max)};" + "\n")
        return ''.join(defs)        

    def _CreateHandlerFunction(self) -> str:
        """Creates the functions and default values for the attributes"""
        struct = []
        parameterNumber = 0
        defineParameter = 0
        for i in range(0, self.toatalFunctions):
            function = f"int {self.handlerFunctionName[i]}(struct mgmt_ctxt *ctxt)\n"
            struct.append(function)
            function = "{" + "\n"
            struct.append(function)

            function = "{:>28}".format("const uint16_t msgID = ")
            struct.append(function)
            function = f"{i};\n"
            struct.append(function)
            function = "{:>23}".format("int readCbor = 0;\n")
            struct.append(function)

            for j in range(0, self.paramSizeList[i]):
                if self.AttributeType[defineParameter] != "char":
                    function = f"     {self.AttributeType[defineParameter]} "
                    struct.append(function)
                    function = f"{self.AttributeSummary[defineParameter]};\n"
                    struct.append(function)
                defineParameter = defineParameter + 1 

            function = "{:>48}".format("const struct cbor_attr_t params_attr[] = {\n")
            #48 is to give it tab pad
            struct.append(function)

            for k in range(0, self.paramSizeList[i]):
                function = "{:>12}".format("{\n")
                struct.append(function)
                function = "{:>34}".format(".attribute = " + '"' + f"{self.AttributeName[parameterNumber]}" + '"' +",\n")
                struct.append(function)
                function = "{:>23}".format(".type = ")
                struct.append(function)
                function = f"{self._GetCborType(self.AttributeType[parameterNumber])},\n"
                struct.append(function)
                function = "{:>21}".format(".addr.")
                struct.append(function)
                if self.AttributeType[parameterNumber] == "char":
                    function = "string = "
                    struct.append(function)
                    function = f"{self.AttributeSummary[parameterNumber]},\n"
                    struct.append(function)
                    function = "{:>29}".format(".len = sizeof(")
                    struct.append(function)
                    function = f"{self.AttributeSummary[parameterNumber]}),\n"
                    struct.append(function)
                elif (self.AttributeType[parameterNumber] == "uint8_t") or (self.AttributeType[parameterNumber] == "uint16_t") or (self.AttributeType[parameterNumber] == "uint32_t"):
                    function = "uinteger = "
                    struct.append(function)
                    function = f"&{self.AttributeSummary[parameterNumber]},\n"
                    struct.append(function)
                else:
                    function = "integer = "
                    struct.append(function)
                    function = f"&{self.AttributeSummary[parameterNumber]},\n"
                    struct.append(function)

                function = "{:>13}".format("},\n")
                struct.append(function)
                parameterNumber = parameterNumber + 1         
            
            function = "{:>8}".format("};\n") #end of params_attr
            struct.append(function)

            function = "{:>58}".format("readCbor = cbor_read_object(&ctxt->it, params_attr);\n")
            struct.append(function)
            function = "{:>26}".format("if (readCbor != 0) {\n")
            struct.append(function)
            function = "{:>34}".format("return MGMT_ERR_EINVAL;\n")
            struct.append(function)
            function = "{:>7}".format("}\n")
            struct.append(function)

            function = self._CreateCborResponse(i)
            struct.append(function)

            function = "}" + "\n" # end of mgmt function
            struct.append(function)
        string = ''.join(struct)
        return string

    def _CreateCborResponse(self, mId: str) -> str:
        response = []

        errorStart = "{:>12}".format("err |= ")
        cborStringsz = "cbor_encode_text_stringz(&ctxt->encoder, "
        cborUint = "cbor_encode_uint(&ctxt->encoder, "

        response.append("{:>24}".format("CborError err = 0;\n"))
        
        response.append(errorStart)
        response.append(cborStringsz)
        response.append('"'+"id" + '");\n')
        response.append(errorStart)
        response.append(cborUint)
        response.append("msgID);\n")
        

        string = ''.join(response)
        return string

    def _CreateHandler(self, category: str) -> str:
        """Creates the structures and default values for Set and Get attributes"""
        struct = []
        for i in range(0, self.toatalFunctions):
            if category in self.indices[i]:
                name = self.functionNames[i]
                result = f"    [{self.mgmtIdPrefex}{name.upper()}] = " +"{" + "\n"
                struct.append(result)
                if category == "_SET":
                    result = f"         .mh_write = {self.handlerFunctionName[i]}" + "\n"
                    struct.append(result)
                else:    
                    result = f"         .mh_read = {self.handlerFunctionName[i]}" + "\n"
                    struct.append(result)
                result = "    }," + "\n"
                struct.append(result)

        string = ''.join(struct)
        return string

    def _CreateAttributeSourceFile(self, lst: list) -> None:
        """Create the settings/attributes/properties *.c file"""
        name = self.outputSourceFileName + ".c"
        print("Writing " + name)
        with open(name, 'w') as fout:
            for index, line in enumerate(lst):
                if "pystart - " in line:
                    if "mgmt handlers" in line:
                        lst.insert(index + 1, self._CreateHandler("_SET"))
                        lst.insert(index + 1, self._CreateHandler("_GET"))  
            fout.writelines(lst)   

    def _CreateMgmtHandlerFile(self, lst: list) -> None:
        """Create the settings/attributes/properties *.c file"""
        name = self.outputSourceHandlerfileName + ".c"
        print("Writing " + name)
        with open(name, 'w') as fout:
            for index, line in enumerate(lst):
                if "pystart - " in line:
                    if "mgmt handlers" in line:
                        lst.insert(index + 1, self._CreateHandlerFunction())
                    elif "string array defines" in line:
                        lst.insert(index + 1, self._CreateStringDefinitions()) 
            fout.writelines(lst)                

    def _CreateAttrIndices(self) -> str:
        """Create attribute indices for header file"""
        for i in range(0, self.toatalFunctions):
            name = self.functionNames[i]
            id = self.functionId[i]
            result = f"#define {self.mgmtIdPrefex}{name.upper():<37} {id}" + "\n"
            self.indices.append(result)
        return ''.join(self.indices)

    def _CreateAttrDefinitions(self) -> str:
        """Create some definitinons for header file"""
        defs = []
        for i in range(0, self.toatalFunctions):
            name = self.functionNames[i]
            self.handlerFunctionName.append(f"{self.fileName}" + "_" + f"{name}")
            defs.append( f"mgmt_handler_fn {self.handlerFunctionName[i]};\n")
        return ''.join(defs)

    def _CreateAttributeHeaderFile(self, lst: list) -> None:
        """Create the attribute header file"""
        name = self.outputHeaderFileName + ".h"
        print("Writing " + name)
        with open(name, 'w') as fout:
            for index,line in enumerate(lst):
                if "pystart - " in line:
                    if "mgmt function indices" in line:
                        lst.insert(index + 1, self._CreateAttrIndices())
                    elif "mgmt handler function defines" in line:
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
    
    
