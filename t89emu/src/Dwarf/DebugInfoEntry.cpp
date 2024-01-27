#include "Dwarf/DebugInfoEntry.h"

DebugData::DebugData(FormEncoding form) : form(form) {}

DebugData::DebugData() : form(DW_FORM_data8) {}

DebugData::~DebugData() {}

void DebugData::write(uint8_t *buff, size_t len) {
    for (size_t i = 0; i < len; i++) {
        data.push_back(buff[i]);
    }
}

bool DebugData::isString() {
    return form == DW_FORM_string || form == DW_FORM_strp ||
           form == DW_FORM_line_strp;
}

const uint8_t *DebugData::getData() { return data.data(); }

FormEncoding DebugData::getForm() { return form; }

uint64_t DebugData::getUInt() {
    uint64_t res = 0;

    switch (form) {
    case DW_FORM_addr:
    case DW_FORM_block:
    case DW_FORM_exprloc:
    case DW_FORM_data1:
    case DW_FORM_flag:
    case DW_FORM_flag_present:
    case DW_FORM_ref1:
    case DW_FORM_strx1:
    case DW_FORM_addrx1:
    case DW_FORM_data2:
    case DW_FORM_ref2:
    case DW_FORM_strx2:
    case DW_FORM_addrx2:
    case DW_FORM_strx3:
    case DW_FORM_addrx3:
    case DW_FORM_data4:
    case DW_FORM_ref_addr:
    case DW_FORM_ref4:
    case DW_FORM_sec_offset:
    case DW_FORM_line_strp:
    case DW_FORM_strx4:
    case DW_FORM_addrx4:
    case DW_FORM_data8:
    case DW_FORM_ref8:
    case DW_FORM_ref_sig8:
    case DW_FORM_sdata:
    case DW_FORM_udata:
    case DW_FORM_ref_udata: 
    case DW_FORM_strx:
    case DW_FORM_addrx:
    case DW_FORM_loclistx:
    case DW_FORM_rnglistx:
    case DW_FORM_implicit_const: {
        for (size_t i = 0; i < data.size(); i++) {
            res |= (uint64_t)data[i] << (8*i);
        }
        break;
    }

    default: // FORM does not support reading as int
        std::cerr << "Unsupported form: " << form << std::endl;
        exit(1);
    }
    return res;
}

size_t DebugData::getLen() { return data.size(); }

const char *DebugData::getString() {
    return (new std::string(data.begin(), data.end()))->c_str();
}

AttributeEntry::AttributeEntry(AttributeEncoding name, FormEncoding form,
                               size_t special)
    : name(name), form(form), special(special) {}

AttributeEntry::~AttributeEntry() {}

AttributeEncoding AttributeEntry::getName() { return name; }

FormEncoding AttributeEntry::getForm() { return form; }

size_t AttributeEntry::getSpecial() { return special; }

AbbrevEntry::AbbrevEntry(size_t dieCode, TagEncoding dieTag, bool hasChild)
    : dieCode(dieCode), dieTag(dieTag), hasChild(hasChild) {}

AbbrevEntry::~AbbrevEntry() {
    for (AttributeEntry *attEntry : attEntries) {
        delete attEntry;
    }

    attEntries.clear();
}

void AbbrevEntry::addAttributeEntry(AttributeEntry *attEntry) {
    attEntries.push_back(attEntry);
}

size_t AbbrevEntry::getNumAttributes(void) { return attEntries.size(); }

AttributeEntry *AbbrevEntry::getAttributeEntry(size_t index) {
    if (index < 0 || index >= attEntries.size()) {
        return nullptr;
    }
    return attEntries[index];
}

size_t AbbrevEntry::getDieCode() { return dieCode; }

TagEncoding AbbrevEntry::getDieTag() { return dieTag; }

bool AbbrevEntry::hasChildren() { return hasChild; }

AbbrevTable::AbbrevTable(uint8_t *abbrevTableStart) {
    abbrevData = new DataStream(abbrevTableStart);

    for (;;) {
        size_t dieCode = abbrevData->decodeULeb128();
        if (dieCode == 0) { // End of Compile Unit
            break;
        }

        TagEncoding dieTag = (TagEncoding)abbrevData->decodeULeb128();
        size_t hasChild = abbrevData->decodeULeb128();
        AbbrevEntry *abbrevEntry = new AbbrevEntry(dieCode, dieTag, hasChild);

        for (;;) {
            AttributeEncoding name =
                (AttributeEncoding)abbrevData->decodeULeb128();
            FormEncoding form = (FormEncoding)abbrevData->decodeULeb128();
            if (name == 0 && form == 0) { break; }
            size_t special = (form == DW_FORM_implicit_const)
                             ? abbrevData->decodeULeb128() : 0;

            abbrevEntry->addAttributeEntry(
                new AttributeEntry(name, form, special));
        }

        abbrevEntries.insert({dieCode, abbrevEntry});
    }
}

AbbrevTable::~AbbrevTable() {
    for (auto const &abbrevEntry : abbrevEntries) {
        delete abbrevEntry.second;
    }
    abbrevEntries.clear();

    delete abbrevData;
}

AbbrevEntry *AbbrevTable::getAbbrevEntry(size_t dieCode) {
    return abbrevEntries.find(dieCode) != abbrevEntries.end()
               ? abbrevEntries[dieCode]
               : nullptr;
}

DebugInfoEntry::DebugInfoEntry(CompileUnit *compileUnit, DebugInfoEntry *parent)
    : compileUnit(compileUnit), parent(parent), code(0) {}

DebugInfoEntry::~DebugInfoEntry() {
    for (DebugInfoEntry *debugEntry : children) {
        delete debugEntry;
    }
}

void DebugInfoEntry::addAttribute(AttributeEncoding encoding, DebugData *data) {
    attributes.insert({encoding, data});
}

void DebugInfoEntry::addChild(DebugInfoEntry *child) {
    children.push_back(child);
}

void DebugInfoEntry::printEntry() {
    printf("\nCode: 0x%02lx %s\n", code, printTag(abbrevEntry->getDieTag()));
    for (size_t i = 0; i < abbrevEntry->getNumAttributes(); i++) {
        // Use abbrevEntry so attributes printed in correct order
        AttributeEntry *attEntry = abbrevEntry->getAttributeEntry(i);
        printf("\t%-30s%-23s", printAttribute(attEntry->getName()),
               printForm(attEntry->getForm()));
        DebugData *data = attributes[attEntry->getName()];
        if (data->isString()) { printf("(%s)\n", data->getString()); }
        else { printf("(%lx)\n", data->getUInt()); }
    }
}

AbbrevEntry *DebugInfoEntry::getAbbrevEntry() { return abbrevEntry; }

DebugData *DebugInfoEntry::getAttribute(AttributeEncoding attribute) {
    return (attributes.find(attribute) != attributes.end())
            ? attributes[attribute] : nullptr;
}

size_t DebugInfoEntry::getCode() { return code; }

TagEncoding DebugInfoEntry::getTag() { return abbrevEntry->getDieTag(); }

size_t DebugInfoEntry::getNumChildren() { return children.size(); }

DebugInfoEntry *DebugInfoEntry::getChild(size_t index) {
    return index < 0 || index >= children.size() ? nullptr : children[index];
}

DebugInfoEntry *DebugInfoEntry::getParent() { return parent; }

void DebugInfoEntry::setAbbrevEntry(AbbrevEntry *abbrevEntry) {
    this->abbrevEntry = abbrevEntry;
}

void DebugInfoEntry::setCode(size_t dieCode) { code = dieCode; }

bool DebugInfoEntry::isScope() {
    switch (abbrevEntry->getDieTag()) {
    case DW_TAG_subprogram:
    case DW_TAG_inlined_subroutine:
    case DW_TAG_entry_point: return true;
    default: return false;
    }
}

// Some types that aren't C/C++ aren't listed here, and some types can only be
// children to the ones below
bool DebugInfoEntry::isType() {
    switch (abbrevEntry->getDieTag()) {
    case DW_TAG_array_type:
    case DW_TAG_atomic_type:
    case DW_TAG_base_type:
    case DW_TAG_class_type:
    case DW_TAG_const_type:
    case DW_TAG_enumeration_type:
    case DW_TAG_pointer_type:
    case DW_TAG_ptr_to_member_type:
    case DW_TAG_reference_type:
    case DW_TAG_restrict_type:
    case DW_TAG_rvalue_reference_type:
    case DW_TAG_string_type:
    case DW_TAG_structure_type:
    case DW_TAG_typedef:
    case DW_TAG_union_type:
    case DW_TAG_unspecified_type:
    case DW_TAG_volatile_type: return true;
    default: return false;
    }
}

bool DebugInfoEntry::isVariable() {
    switch (abbrevEntry->getDieTag()) {
    case DW_TAG_variable:
    case DW_TAG_formal_parameter: return true;
    default: return false;
    }
}