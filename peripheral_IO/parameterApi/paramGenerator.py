import json
import openpyxl
import collections
import os


def toyn(b) -> str:
    if b:
        return "y"
    else:
        return "n"


class attributes:
    def __init__(self, fname: str = "BT6ApiParams.json"):

        # The following items are loaded from the configuration file
        self.componentList = 0
        self.totalParameters = 0

        self.headerFilePath = "../../common/include/"
        self.sourceFilePath = "../../common/source/"
        self.fileName = "AttributeTable"

        self.inputHeaderFileName = ""
        self.outputHeaderFileName = ""
        self.inputSourceFileName = ""
        self.outputSourceFileName = ""
        self.jsonFileName = ""

        self.functionCategory = []
        self.ParamNames = []
        self.ParamId = []
        self.AttributeMax = []
        self.AttributeMin = []
        self.AttributeName = []
        self.AttributeSummary = []
        self.AttributeType = []
        self.AttributeLockable = []
        self.AttributeBroadcast = []
        self.AttributeStringMax = []
        self.AttributeDefault = []
        self.AttributeWritable = []
        self.AttributeReadable = []
        self.AttributeSavable = []
        self.AttributeDeprecated = []
        self.AttributeValidator = []
        self.AttributePrepare = []
        self.resultName = []
        self._LoadConfig(fname)

    def _GetNumberField(self, index, item: str):
        """ Handles items not being present (lazy get) """
        try:
            r = self.componentList[index]['schema'][item]
            return r
        except:
            return 0.0

    def _GetBoolField(self, index, item: str):
        try:
            r = self.componentList[index]['schema'][item]
            return r
        except:
            return False

    def _GetStringField(self, index, item: str):
        try:
            r = self.componentList[index]['schema'][item]
            return r
        except:
            return ""

    def _LoadConfig(self, fname: str) -> None:
        with open(fname, 'r') as f:
            data = json.load(f)
            self.componentList = data['components']['contentDescriptors']['deviceParams']['x-deviceparameters']
            self.totalParameters = len(self.componentList)
            file_name = self.headerFilePath + self.fileName
            self.inputHeaderFileName = file_name
            self.outputHeaderFileName = file_name + ""
            file_name = self.sourceFilePath + self.fileName
            self.inputSourceFileName = file_name
            self.outputSourceFileName = file_name + ""

            for i in range(self.totalParameters):
                self.ParamNames.append(self.componentList[i]['name'])
                self.ParamId.append(self.componentList[i]['x-id'])
                # schema sub
                self.AttributeStringMax.append(
                    self._GetNumberField(i, 'maximumlength'))
                self.AttributeMax.append(self._GetNumberField(i, 'maximum'))
                self.AttributeMin.append(self._GetNumberField(i, 'minimum'))
                self.AttributeDefault.append(
                    self.componentList[i]['schema']['x-default'])
                self.AttributeType.append(
                    self.componentList[i]['schema']['x-ctype'])
                self.AttributeLockable.append(
                    self._GetBoolField(i, 'x-lockable'))
                self.AttributeBroadcast.append(
                    self._GetBoolField(i, 'x-broadcast'))
                self.AttributeReadable.append(
                    self._GetBoolField(i, 'x-readable'))
                self.AttributeWritable.append(
                    self._GetBoolField(i, 'x-writeable'))
                self.AttributeSavable.append(
                    self._GetBoolField(i, 'x-savable'))
                self.AttributeDeprecated.append(
                    self._GetBoolField(i, 'x-deprecated'))
                self.AttributeValidator.append(
                    self._GetStringField(i, 'x-validator'))
                self.AttributePrepare.append(
                    self._GetBoolField(i, 'x-prepare'))
                # todo: max string length
            pass

    def _GetType(self, itype: str) -> str:
        if itype == "string":
            return "s  "
        elif itype == "float":
            return "f  "
        elif itype == "int8_t":
            return "i8 "
        elif itype == "int16_t":
            return "i16"
        elif itype == "int32_t":
            return "i32"
        elif itype == "uint8_t":
            return "u8 "
        elif itype == "uint16_t":
            return "u16"
        elif itype == "uint32_t":
            return "u32"
        elif itype == "int64_t":
            return "i64"
        elif itype == "uint64_t":
            return "u64"
        else:  # u8 array
            return "a  "

    def _GetAttributeMacro(self, itype: str, savable: bool, name: str) -> str:
        """Get the c-macro for the RW or RO attribute (array vs non-array pointer)"""
        if itype == "string":
            if savable:
                return "RW_ATTRS(" + name + ")"
            else:
                return "RO_ATTRS(" + name + ")"
        else:
            if savable:
                return "RW_ATTRX(" + name + ")"
            else:
                return "RO_ATTRX(" + name + ")"

    def _GetValidatorString(self, i_type: str, index: int) -> str:
        """Use custom validator if it exists.  Otherwise use validator based on type"""
        validator = self.AttributeValidator[index]
        if validator != "":
            return "AttributeValidator_" + validator
        elif i_type == "string":
            return "AttributeValidator_" + "string"
        else:
            return "AttributeValidator_" + (i_type).replace('_t', '')

    def _CreateMinMaxString(self, imin: str, imax: str, i_type: str) -> str:
        """Create the min/max portion of the attribute table entry"""
        if i_type == "string":
            # string validation is different and doesn't use min/max
            s_min = ".min.ux = 0"
            s_max = ".max.ux = 0"
        elif i_type == "float":
            s_min = ".min.fx = " + str(imin)
            s_max = ".max.fx = " + str(imax)
        else:
            if int(imin) < 0 or int(imax) < 0:
                s_min = f".min.sx = " + str(imin)
                s_max = f".max.sx = " + str(imax)
            else:
                s_min = f".min.ux = " + str(imin)
                s_max = f".max.ux = " + str(imax)

        return s_min.ljust(20) + ", " + s_max.ljust(20)

    def _CreateAttrTable(self) -> str:
        """
        Create the attribute (property) table from the dictionary of lists
        created from the Excel spreadsheet and gperf
        """
        attributeTable = []
        for i in range(0, self.totalParameters):
            name = self.ParamNames[i]
            i_type = self.AttributeType[i]
            i_min = self.AttributeMin[i]
            i_max = self.AttributeMax[i]
            writable = toyn(self.AttributeWritable[i])
            readable = toyn(self.AttributeReadable[i])
            lockable = toyn(self.AttributeLockable[i])
            broadcast = toyn(self.AttributeBroadcast[i])
            savable = toyn(self.AttributeSavable[i])
            deprecated = toyn(self.AttributeDeprecated[i])
            i_hash = i
            result = f"    [{i_hash:<3}] = " \
                + "{ " \
                + f"{self._GetAttributeMacro(i_type, self.AttributeSavable[i], name):<40}, {self._GetType(i_type)}, {savable}, {writable}, {readable}, {lockable}, {broadcast}, {deprecated}, {self._GetValidatorString(i_type, i):<28}, {self._GetPrepareString(name, i)}, {self._CreateMinMaxString(i_min, i_max, i_type)}" \
                + " }," \
                + "\n"
            attributeTable.append(result)

        attributeTable.append("\n")

        string = ''.join(attributeTable)
        return string[:string.rfind(',')] + '\n'

    def _GetStringSize(self, itype: str, imax: str) -> str:
        if itype == "char":
            return f"[{imax} + 1]"  # add one for the NUL character
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

    def _GetPrepareString(self, name: str, index: int) -> str:
        if self.AttributePrepare[index]:
            s = "AttributePrepare_" + name
        else:
            s = "NULL"

        return s.ljust(42)

    def UpdateFiles(self) -> None:
        """Update the attribute c/h files.
        minHashStringLength is specific to each hash to prevent out-of-bounds array index.
        The suffix can be used to prevent the input files from being overwritten"""
        # self._CheckLists()
        self._CreateAttributeSourceFile(
            self._CreateInsertionList(self.inputSourceFileName + ".c"))
        self._CreateAttributeHeaderFile(
            self._CreateInsertionList(self.inputHeaderFileName + ".h"))

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
        """
        Creates the structures and default values for RW and RO attributes.
        Writable but non-savable values are populated in the RO structure.
        """
        struct = []
        for i in range(0, self.totalParameters):
            savable = self.AttributeSavable[i]
            writable = self.AttributeWritable[i]
            readable = self.AttributeReadable[i]
            if (category == 'rw' and (writable or savable)) or (
                    (category == 'ro') and not savable):
                name = self.ParamNames[i]
                # string is required in test tool, c requires char type
                if self.AttributeType[i] == "string":
                    i_type = "char"
                else:
                    i_type = self.AttributeType[i]
                i_max = self.AttributeStringMax[i]
                default = self.AttributeDefault[i]
                if default_values:
                    result = f"    .{name} = {self._GetDefault(i_type, default)}," + "\n"
                else:
                    result = f"    {i_type} {name}{self._GetStringSize(i_type, i_max)};" + "\n"
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
                    elif "rw attributes" in line:
                        lst.insert(
                            index + 1, self._CreateStruct("rw", False, False))
                    elif "rw defaults" in line:
                        lst.insert(
                            index + 1, self._CreateStruct("rw", True, True))
                    elif "ro attributes" in line:
                        lst.insert(
                            index + 1, self._CreateStruct("ro", False, True))
                    elif "ro defaults" in line:
                        lst.insert(
                            index + 1, self._CreateStruct("ro", True, True))
                    elif "prepare for read weak implementations":
                        lst.insert(index + 1, self._CreatePrepare(False))

            fout.writelines(lst)

    def _CreateAttrIndices(self) -> str:
        """Create attribute indices for header file"""
        indices = []
        for i in range(0, self.totalParameters):
            name = self.ParamNames[i]
            result = f"#define ATTR_INDEX_{name:<37} {i}" + "\n"
            indices.append(result)
        return ''.join(indices)

    def _CreateTableSize(self) -> str:
        """Create some definitinons for header file"""
        defs = []
        defs.append(f"#define ATTR_TABLE_SIZE {self.totalParameters}\n\n")
        return ''.join(defs)

    def _GeneratePrepareFunctionName(self, index: int) -> str:
        return "int AttributePrepare_" + self.ParamNames[index] + "(void)"

    def _CreatePrepare(self, prototype: bool) -> str:
        """Create prototypes or weak implementation for prepare to read functions"""
        lst = []
        for i in range(0, self.totalParameters):
            if self.AttributePrepare[i]:
                s = self._GeneratePrepareFunctionName(i)
                if prototype:
                    s += ";\n"
                else:
                    s = "__weak " + s + "\n{\n\treturn 0;\n}\n\n"

                lst.append(s)

        return ''.join(lst)

    def _CreateAttributeHeaderFile(self, lst: list) -> None:
        """Create the attribute header file"""
        name = self.outputHeaderFileName + ".h"
        print("Writing " + name)
        with open(name, 'w') as fout:
            for index, line in enumerate(lst):
                if "pystart - " in line:
                    if "attribute indices" in line:
                        lst.insert(index + 1, self._CreateAttrIndices())
                    elif "attribute table size" in line:
                        lst.insert(index + 1, self._CreateTableSize())
                    elif "prepare for read" in line:
                        lst.insert(
                            index + 1, self._CreatePrepare(True))

            fout.writelines(lst)


if __name__ == "__main__":
    a = attributes()
    # a.ImportWorkbook()
    # a.UpdateHash()
    a.UpdateFiles()
    # a.SaveAttributesJson()
    # test loading from file
    # b = attributes()
    # b.LoadAttributesJson()
    # if a == b:
    #    print("match")
    # else:
    #    print("boo")