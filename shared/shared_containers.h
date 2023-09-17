#pragma once

#include "shared_defines.h"

// template<typename _type_>
// void Swap(_type_ & a, _type_ & b) {
//     _type_ tmp = a;
//     a = b;
//     b = tmp;
// }

// template<typename _type_>
// inline void ZeroStruct(_type_ & t) {
//     memset(&t, 0, sizeof(_type_));
// }

// template<typename _type_>
// inline void ZeroStructPtr(_type_ * t) {
//     memset(t, 0, sizeof(_type_));
// }

template<typename T, i32 capcity>
class FixedList {
public:
    T * GetData();
    const T * GetData() const;
    i32             GetCapcity() const;
    i32             GetCount() const;
    void            SetCount(i32 count);
    bool            IsFull() const;
    bool            IsEmpty() const;
    void            Clear(bool zeroMemory = false);

    T & First();
    T & Last();

    T * Add(const T & value);
    T & AddEmpty();
    T * AddUnique(const T & value);
    b8              AddIfPossible(const T & t);

    void            RemoveIndex(const i32 & index);
    void            Remove(const T * ptr);
    void            RemoveValue(const T & value);
    void            Reverse();

    void            Append(const T * t, i32 count);  // Assumes there will be enough space, otherwise will assert
    template<i32 c>
    inline void     Append(const FixedList<T, c> & list) { Append(list.data, list.count); }

    T * Get(const i32 & index);
    const T * Get(const i32 & index) const;

    T & operator[](const i32 & index);
    T operator[](const i32 & index) const;

private:
    T data[capcity];
    i32 count;
};

template<typename T, i32 capcity>
const T * FixedList<T, capcity>::Get(const i32 & index) const {
    AssertMsg(index >= 0 && index < capcity, "Array, invalid index");
    return &data[index];
}


template<typename T, i32 capcity>
T * FixedList<T, capcity>::Get(const i32 & index) {
    AssertMsg(index >= 0 && index < capcity, "Array, invalid index");

    return &data[index];
}

template<typename T, i32 capcity>
void FixedList<T, capcity>::Clear(bool zeroMemory) {
    count = 0;
    if (zeroMemory) {
        ZeroStruct(*this);
    }
}

template<typename T, i32 capcity>
T & FixedList<T, capcity>::Last() {
    AssertMsg(count >= 1, "Array, invalid index");
    return data[count - 1];
}

template<typename T, i32 capcity>
T & FixedList<T, capcity>::First() {
    return data[0];
}

template<typename T, i32 capcity>
bool FixedList<T, capcity>::IsEmpty() const {
    return count == 0;
}

template<typename T, i32 capcity>
bool FixedList<T, capcity>::IsFull() const {
    return capcity == count;
}

template<typename T, i32 capcity>
void FixedList<T, capcity>::SetCount(i32 count) {
    this->count = count;
}

template<typename T, i32 capcity>
i32 FixedList<T, capcity>::GetCount() const {
    return count;
}

template<typename T, i32 capcity>
i32 FixedList<T, capcity>::GetCapcity() const {
    return capcity;
}

template<typename T, i32 capcity>
T * FixedList<T, capcity>::GetData() {
    return data;
}

template<typename T, i32 capcity>
const T * FixedList<T, capcity>::GetData() const {
    return data;
}

template<typename T, i32 capcity>
T * FixedList<T, capcity>::Add(const T & value) {
    i32 index = count; count++;
    AssertMsg(index >= 0 && index < capcity, "Array, add to many items");

    data[index] = value;

    return &data[index];
}

template<typename T, i32 capcity>
T & FixedList<T, capcity>::AddEmpty() {
    i32 index = count; count++;
    AssertMsg(index >= 0 && index < capcity, "Array, add to many items");

    ZeroStruct(data[index]);

    return data[index];
}

template<typename T, i32 capcity>
T * FixedList<T, capcity>::AddUnique(const T & value) {
    for (i32 index = 0; index < count; index++) {
        if (data[index] == value) {
            return &data[index];
        }
    }

    return Add(value);
}

template<typename T, i32 capcity>
b8 FixedList<T, capcity>::AddIfPossible(const T & t) {
    i32 index = count;
    if (index < capcity) {
        data[index] = t;
        count++;

        return true;
    }

    return false;
}

template<typename T, i32 capcity>
void FixedList<T, capcity>::RemoveIndex(const i32 & index) {
    AssertMsg(index >= 0 && index < count, "Array invalid remove index ");
    for (i32 i = index; i < count - 1; i++) {
        data[i] = data[i + 1];
    }
    count--;
}

template<typename T, i32 capcity>
void FixedList<T, capcity>::Remove(const T * ptr) {
    for (i32 i = 0; i < count; i++) {
        if (ptr == &data[i]) {
            RemoveIndex(i);
            return;
        }
    }
}

template<typename T, i32 capcity>
void FixedList<T, capcity>::RemoveValue(const T & value) {
    for (i32 i = 0; i < count; i++) {
        if (value == data[i]) {
            RemoveIndex(i);
            return;
        }
    }
}

template<typename T, i32 capcity>
void FixedList<T, capcity>::Reverse() {
    i32 start = 0;
    i32 end = count - 1;
    while (start < end) {
        Swap(data[start], data[end]);
        start++;
        end--;
    }
}

template<typename T, i32 capcity>
void FixedList<T, capcity>::Append(const T * t, i32 count) {
    AssertMsg(this->count + count <= capcity, "Array, append to many items");

    if (this->count + count <= capcity) {
        for (i32 i = this->count; i < this->count + count; i++) {
            data[i] = t[i - this->count];
        }
        this->count += count;
    }
}

template<typename T, i32 capcity>
T FixedList<T, capcity>::operator[](const i32 & index) const {
    AssertMsg(index >= 0 && index < capcity, "Array, invalid index");

    return data[index];
}

template<typename T, i32 capcity>
T & FixedList<T, capcity>::operator[](const i32 & index) {
    AssertMsg(index >= 0 && index < capcity, "Array, invalid index");

    return data[index];
}

template<typename _type_, i32 capcity>
class FixedFreeList {
public:
    _type_ & Add();
    _type_ & Add(i32 & index);
    void            Remove(_type_ * ptr);
    void            Clear();

    void            GetList(FixedList<_type_ *, capcity> & list);

    i32             GetCount() const;
    i32             GetCapcity() const;

    _type_ & operator[](i32 index);
    const _type_ & operator[](i32 index) const;

private:
    void            InitializedIfNeedBe();
    i32             FindFreeIndex();

    FixedList<_type_, capcity> list;
    FixedList<i32, capcity> freeList;
};

template<typename _type_, i32 capcity>
_type_ & FixedFreeList<_type_, capcity>::Add() {
    i32 _;
    return Add(_);
}

template<typename _type_, i32 capcity>
_type_ & FixedFreeList<_type_, capcity>::Add(i32 & index) {
    InitializedIfNeedBe();
    const i32 freeIndex = FindFreeIndex();
    AssertMsg(freeIndex != -1, "FixedFreeList, no free index");
    list.SetCount(list.GetCount() + 1);
    index = freeIndex;
    return list[freeIndex];
}

template<typename _type_, i32 capcity>
void FixedFreeList<_type_, capcity>::Remove(_type_ * ptr) {
    AssertMsg(ptr >= list.GetData() && ptr < list.GetData() + list.GetCount(), "FixedFreeList, invalid remove ptr");
    const i32 index = (i32)(ptr - list.GetData());
    AssertMsg(index >= 0 && index < list.GetCount(), "FixedFreeList, invalid remove index");
    freeList.Add(index);
}

template<typename _type_, i32 capcity>
void FixedFreeList<_type_, capcity>::Clear() {
    ZeroStruct(freeList);
    ZeroStruct(list);
    InitializedIfNeedBe();
}

template<typename _type_, i32 capcity>
void FixedFreeList<_type_, capcity>::GetList(FixedList<_type_ *, capcity> & inList) {
    FixedList<b8, capcity> activeList = {};//*MemoryAllocateTransient<FixedList<b8, capcity>>();
    for (i32 i = 0; i < capcity; i++) {
        activeList[i] = true;
    }

    const i32 freeCount = freeList.GetCount();
    for (i32 i = 0; i < freeCount; i++) {
        activeList[freeList[i]] = false;
    }

    const i32 activeCount = capcity - freeCount;

    i32 count = 0;
    for (i32 i = 0; i < capcity; i++) {
        if (activeList[i]) {
            _type_ * t = list.Get(i);
            inList.Add(t);
            count++;
        }
        if (count == activeCount) {
            break;
        }
    }
}

template<typename _type_, i32 capcity>
i32 FixedFreeList<_type_, capcity>::GetCount() const {
    return list.GetCount();
}

template<typename _type_, i32 capcity>
i32 FixedFreeList<_type_, capcity>::GetCapcity() const {
    return capcity;
}

template<typename _type_, i32 capcity>
_type_ & FixedFreeList<_type_, capcity>::operator[](i32 index) {
    return list[index];
}

template<typename _type_, i32 capcity>
const _type_ & FixedFreeList<_type_, capcity>::operator[](i32 index) const {
    return list[index];
}

template<typename _type_, i32 capcity>
void FixedFreeList<_type_, capcity>::InitializedIfNeedBe() {
    if (freeList.GetCount() == 0 && list.GetCount() == 0) {
        const i32 cap = freeList.GetCapcity();
        for (i32 i = cap - 1; i >= 0; i--) {
            freeList.Add(i);
        }
    }
}

template<typename _type_, i32 capcity>
i32 FixedFreeList<_type_, capcity>::FindFreeIndex() {
    const i32 freeListCount = freeList.GetCount();
    if (freeListCount > 0) {
        const i32 freeIndex = freeList[freeListCount - 1];
        freeList.RemoveIndex(freeListCount - 1);
        return freeIndex;
    }

    return -1;
}

class StringHash {
public:
    inline static constexpr u64 ConstStrLen(const char * str) {
        u64 len = 0;
        while (str[len] != '\0') {
            len++;
        }

        return len;
    }

    //https://www.partow.net/programming/hashfunctions/index.html
    inline static constexpr u32 DEKHash(const char * str, unsigned int length) {
        unsigned int hash = length;
        unsigned int i = 0;

        for (i = 0; i < length; ++str, ++i) {
            hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
        }

        return hash;
    }

    inline static constexpr u32 Hash(const char * str) {
        return DEKHash(str, (u32)ConstStrLen(str));
    }
};

template<u64 SizeBytes>
class FixedStringBase {
public:
    static constexpr i32 MAX_NUMBER_SIZE = 22;
    static constexpr i32 CAPCITY = static_cast<i32>(SizeBytes - sizeof(i32));

    static FixedStringBase<SizeBytes> constexpr FromLiteral(const char * str);

    void                                SetLength(const i32 & l);
    i32                                 GetLength() const;
    const char * GetCStr() const;
    char * GetCStr();
    void                                CalculateLength();
    void                                Clear();
    FixedStringBase<SizeBytes> & Add(const char & c);
    FixedStringBase<SizeBytes> & Add(const char * c);
    FixedStringBase<SizeBytes> & Add(const FixedStringBase<SizeBytes> & c);
    i32                                 FindFirstOf(const char & c) const;
    i32                                 FindLastOf(const char & c) const;
    i32                                 NumOf(const char & c) const;
    FixedStringBase<SizeBytes>          SubStr(i32 fromIndex) const;
    FixedStringBase<SizeBytes>          SubStr(i32 startIndex, i32 endIndex) const;
    void                                Replace(const char & c, const char & replaceWith);
    void                                RemoveCharacter(const i32 & removeIndex);
    void                                RemoveWhiteSpace();
    void                                RemovePathPrefix(const char * prefix);
    b32                                 Contains(const char * str) const;
    b32	                                Contains(const FixedStringBase & str) const;
    b32	                                StartsWith(const FixedStringBase & str) const;
    b32	                                EndsWith(const char * str) const;
    b32	                                EndsWith(const FixedStringBase & str) const;
    void                                CopyFrom(const FixedStringBase & src, const i32 & start, const i32 & end);
    void                                ToUpperCase();
    void                                StripFileExtension();
    void                                StripFilePath();
    void                                BackSlashesToSlashes();
    //TransientList<FixedStringBase>      Split(char delim) const;

    FixedStringBase<SizeBytes> & operator=(const char * other);

    bool                                operator==(const char * other);
    bool                                operator==(const FixedStringBase<SizeBytes> & other) const;
    char & operator[](const i32 & index);
    char                                operator[](const i32 & index) const;
    bool                                operator!=(const FixedStringBase<SizeBytes> & other) const;

private:
    i32 length;
    char data[CAPCITY];
};

template<u64 SizeBytes>
FixedStringBase<SizeBytes> constexpr FixedStringBase<SizeBytes>::FromLiteral(const char * str) {
    FixedStringBase<SizeBytes> result = {};
    result.Add(str);
    return result;
}

//template<u64 SizeBytes>
//FixedStringBase<SizeBytes>::FixedStringBase(const char* str) {

//}

template<u64 SizeBytes>
void FixedStringBase<SizeBytes>::SetLength(const i32 & l) {
    length = l;
}

template<u64 SizeBytes>
i32 FixedStringBase<SizeBytes>::GetLength() const {
    return length;
}

template<u64 SizeBytes>
const char * FixedStringBase<SizeBytes>::GetCStr() const {
    return data;
}

template<u64 SizeBytes>
char * FixedStringBase<SizeBytes>::GetCStr() {
    return data;
}

template<u64 SizeBytes>
void FixedStringBase<SizeBytes>::CalculateLength() {
    length = static_cast<i32>(StringHash::ConstStrLen(data));
}

template<u64 SizeBytes>
void FixedStringBase<SizeBytes>::Clear() {
    length = 0;
    for (i32 i = 0; i < CAPCITY; i++) {
        data[i] = '\0';
    }
}

template<u64 SizeBytes>
FixedStringBase<SizeBytes> & FixedStringBase<SizeBytes>::Add(const char & c) {
    i32 index = length;
    index++;

    AssertMsg(index < CAPCITY, "FixedStringBase, to many characters");

    if (index < CAPCITY) {
        length = index;
        data[index - 1] = c;
    }

    return *this;
}

template<u64 SizeBytes>
FixedStringBase<SizeBytes> & FixedStringBase<SizeBytes>::Add(const char * c) {
    i32 index = length;
    AssertMsg(index + 1 < CAPCITY, "FixedStringBase, to many characters");

    for (i32 i = 0; index < CAPCITY && c[i] != '\0'; i++, index++) {
        AssertMsg(index < CAPCITY, "FixedStringBase, to many characters");

        if (index < CAPCITY) {
            data[index] = c[i];
            length = index + 1;
        }
    }

    return *this;
}

template<u64 SizeBytes>
FixedStringBase<SizeBytes> & FixedStringBase<SizeBytes>::Add(const FixedStringBase<SizeBytes> & c) {
    return Add(c.data);
}

template<u64 SizeBytes>
i32 FixedStringBase<SizeBytes>::FindFirstOf(const char & c) const {
    const i32 l = length;
    for (i32 i = 0; i < l; i++) {
        if (data[i] == c) {
            return i;
        }
    }

    return -1;
}

template<u64 SizeBytes>
i32 FixedStringBase<SizeBytes>::FindLastOf(const char & c) const {
    const i32 l = length;
    for (i32 i = l; i >= 0; i--) {
        if (data[i] == c) {
            return i;
        }
    }

    return -1;
}

template<u64 SizeBytes>
i32 FixedStringBase<SizeBytes>::NumOf(const char & c) const {
    i32 count = 0;
    for (i32 i = 0; i < length; i++) {
        if (data[i] == c) {
            count++;
        }
    }

    return count;
}

template<u64 SizeBytes>
FixedStringBase<SizeBytes> FixedStringBase<SizeBytes>::SubStr(i32 fromIndex) const {
    const i32 l = length;
    AssertMsg(fromIndex >= 0 && fromIndex < l, "SubStr range invalid");

    FixedStringBase<SizeBytes> result = "";
    for (i32 i = fromIndex; i < l; i++) {
        result.Add(data[i]);
    }

    return result;
}

template<u64 SizeBytes>
FixedStringBase<SizeBytes> FixedStringBase<SizeBytes>::SubStr(i32 startIndex, i32 endIndex) const {
    const i32 l = length;
    AssertMsg(startIndex >= 0 && startIndex < l, "SubStr range invalid");
    AssertMsg(endIndex >= 0 && endIndex < l, "SubStr range invalid");
    AssertMsg(startIndex < endIndex, "SubStr range invalid");

    FixedStringBase<SizeBytes> result = "";
    for (i32 i = startIndex; i < endIndex; i++) {
        result.Add(data[i]);
    }

    return result;
}


template<u64 SizeBytes>
void FixedStringBase<SizeBytes>::Replace(const char & c, const char & replaceWith) {
    const i32 l = length;
    for (i32 i = 0; i < l; i++) {
        if (data[i] == c) {
            data[i] = replaceWith;
        }
    }
}

template<u64 SizeBytes>
void FixedStringBase<SizeBytes>::RemoveCharacter(const i32 & removeIndex) {
    const i32 l = length;
    AssertMsg(removeIndex >= 0 && removeIndex < l, "FixedStringBase, invalid index");

    for (i32 i = removeIndex; i < l; i++) {
        data[i] = data[i + 1];
    }

    length = l - 1;
}

template<u64 SizeBytes>
void FixedStringBase<SizeBytes>::RemoveWhiteSpace() {
    for (i32 i = 0; i < GetLength(); i++) {
        char d = data[i];
        if (d == ' ' || d == '\n' || d == '\t' || d == '\r') {
            RemoveCharacter(i);
            i--;
        }
    }
}

template<u64 SizeBytes>
void FixedStringBase<SizeBytes>::RemovePathPrefix(const char * prefix) {
    const i32 l = length;
    const i32 prefixLength = static_cast<i32>(StringHash::ConstStrLen(prefix));

    AssertMsg(prefixLength <= l, "FixedStringBase, invalid prefix");

    for (i32 i = 0; i < prefixLength; i++) {
        if (data[i] != prefix[i]) {
            return;
        }
    }

    for (i32 i = 0; i + prefixLength < l; i++) {
        data[i] = data[i + prefixLength];
    }

    length = l - prefixLength;

    for (i32 i = length; i < CAPCITY; i++) {
        data[i] = '\0';
    }
}

template<u64 SizeBytes>
b32 FixedStringBase<SizeBytes>::Contains(const char * str) const {
    const i32 l = length;
    const i32 strLen = static_cast<i32>(StringHash::ConstStrLen(str));

    for (i32 i = 0; i < l; i++) {
        if (data[i] == str[0]) {
            b32 match = true;
            for (i32 j = 0; j < strLen; j++) {
                if (data[i + j] != str[j]) {
                    match = false;
                    break;
                }
            }

            if (match) {
                return true;
            }
        }
    }

    return false;
}

template<u64 SizeBytes>
b32 FixedStringBase<SizeBytes>::Contains(const FixedStringBase & str) const {
    const i32 otherLength = str.length;
    const i32 ourLength = length;
    b32 result = false;

    for (i32 i = 0; i < ourLength && !result; i++) {
        result = true;
        for (i32 j = 0; j < otherLength; j++) {
            if (data[i] != str.data[j]) {
                result = false;
                break;
            }
            else {
                i++;
            }
        }
    }

    return result;
}

template<u64 SizeBytes>
b32 FixedStringBase<SizeBytes>::StartsWith(const FixedStringBase & str) const {
    const i32 l = length;
    const i32 ll = str.length;

    if (l < ll) {
        return false;
    }

    for (i32 i = 0; i < ll; i++) {
        if (data[i] != str.data[i]) {
            return false;
        }
    }

    return true;
}

template<u64 SizeBytes>
b32 FixedStringBase<SizeBytes>::EndsWith(const char * str) const {
    const i32 l = length;
    const i32 ll = static_cast<i32>(StringHash::ConstStrLen(str));;

    if (l < ll) {
        return false;
    }

    for (i32 i = 0; i < ll; i++) {
        if (data[l - i - 1] != str[ll - i - 1]) {
            return false;
        }
    }

    return true;
}

template<u64 SizeBytes>
b32 FixedStringBase<SizeBytes>::EndsWith(const FixedStringBase & str) const {
    return EndsWith(str.data);
}

template<u64 SizeBytes>
void FixedStringBase<SizeBytes>::CopyFrom(const FixedStringBase & src, const i32 & start, const i32 & end) {
    AssertMsg(start >= 0 && start < src.length, "FixedStringBase, invalid index");
    AssertMsg(end >= 0 && end < src.length, "FixedStringBase, invalid index");

    i32 writePtr = 0;
    for (i32 i = start; i <= end; i++, writePtr++) {
        data[writePtr] = src.data[i];
    }

    length = writePtr;
}

template<u64 SizeBytes>
void FixedStringBase<SizeBytes>::ToUpperCase() {
    const i32 l = length;
    for (i32 i = 0; i < l; i++) {
        data[i] = (char)toupper(data[i]);
    }
}

template<u64 SizeBytes>
void FixedStringBase<SizeBytes>::StripFileExtension() {
    i32 index = FindLastOf('.');
    if (index > 0) {
        for (i32 i = index; i < length; i++) {
            data[i] = '\0';
        }
    }

    length = (i32)StringHash::ConstStrLen((char *)data);
}

template<u64 SizeBytes>
void FixedStringBase<SizeBytes>::StripFilePath() {
    i32 index = FindLastOf('/');

    if (index > 0) {
        i32 cur = 0;
        for (i32 i = index + 1; i < length; i++) {
            data[cur] = data[i];
            cur++;
        }

        for (i32 i = cur; i < length; i++) {
            data[i] = '\0';
        }

        length = (i32)StringHash::ConstStrLen((char *)data);
    }
}

template<u64 SizeBytes>
void FixedStringBase<SizeBytes>::BackSlashesToSlashes() {
    for (i32 i = 0; i < length; i++) {
        if (data[i] == '\\') {
            data[i] = '/';
        }
    }
}

//template<u64 SizeBytes>
//TransientList<FixedStringBase<SizeBytes>> FixedStringBase<SizeBytes>::Split(char delim) const {
//    const i32 num = NumOf(delim) + 1;
//    TransientList<FixedStringBase<SizeBytes>> result(
//        num,
//        Memory::AllocateTransientStruct<FixedStringBase<SizeBytes>>(num)
//    );

//    i32 start = 0;
//    i32 end = 0;
//    const i32 len = GetLength();
//    for (; end < len; end++) {
//        if (data[end] == delim) {
//            if (start != end) {
//                result[result.count].CopyFrom(*this, start, end - 1);
//                result.count++;
//                start = end + 1;
//            }
//            else {
//                start++;
//            }
//        }
//    }

//    if (end != start) {
//        result[result.count].CopyFrom(*this, start, end - 1);
//        result.count++;
//    }

//    return result;
//}

template<u64 SizeBytes>
FixedStringBase<SizeBytes> & FixedStringBase<SizeBytes>::operator=(const char * other) {
    *this = FromLiteral(other);
    return *this;
}

template<u64 SizeBytes>
bool FixedStringBase<SizeBytes>::operator==(const char * other) {
    i32 index = 0;
    const i32 l = length;
    const i32 o = static_cast<i32>(StringHash::ConstStrLen(other));

    while (index < l) {
        if (index >= o || data[index] != other[index]) {
            return false;
        }
        index++;
    }

    return true;
}

template<u64 SizeBytes>
bool FixedStringBase<SizeBytes>::operator==(const FixedStringBase<SizeBytes> & other) const {
    const i32 l1 = length;
    const i32 l2 = other.length;

    if (l1 != l2) {
        return false;
    }

    for (i32 i = 0; i < l1; i++) {
        if (data[i] != other.data[i]) {
            return false;
        }
    }

    return true;
}

template<u64 SizeBytes>
char & FixedStringBase<SizeBytes>::operator[](const i32 & index) {
    AssertMsg(index >= 0 && index < length, "FixedString, invalid index");

    return data[index];
}

template<u64 SizeBytes>
char FixedStringBase<SizeBytes>::operator[](const i32 & index) const {
    AssertMsg(index >= 0 && index < length, "FixedString, invalid index");
    return data[index];
}

template<u64 SizeBytes>
bool FixedStringBase<SizeBytes>::operator!=(const FixedStringBase<SizeBytes> & other) const {
    return !(*this == other);
}

typedef FixedStringBase<64>           SmallString;
typedef FixedStringBase<256>          LargeString;
//typedef FixedStringBase<Megabytes(4)> VeryLargeString;

class StringFormat {
public:
    static SmallString Small(const char * format, ...);
    static LargeString Large(const char * format, ...);
};

template<typename T, i32 capcity>
class FixedQueue {
public:
    bool            IsEmpty() const;
    void            Clear(bool zeroOut = false);
    T *             Enqueue(const T & value);
    T               Dequeue();
    T *             Peek();
    bool            Contains(const T & value);
    inline i32      GetCount() { return count; }

private:
    T data[capcity];
    i32 count;
    i32 head;
    i32 tail;
};

template<typename T, i32 capcity>
bool FixedQueue<T, capcity>::IsEmpty() const {
    return count == 0;
}

template<typename T, i32 capcity>
void FixedQueue<T, capcity>::Clear(bool zeroOut) {
    count = 0;
    head = 0;
    tail = 0;
    if (zeroOut) {
        MemorySet(data, 0, sizeof(T) * capcity);
    }
}

template<typename T, i32 capcity>
T * FixedQueue<T, capcity>::Enqueue(const T & value) {
    AssertMsg(count < capcity, "FixedQueue, queue is full");

    if (count == capcity) {
        return nullptr;
    }

    data[tail] = value;
    T * stored = &data[tail];
    tail = (tail + 1) % capcity;
    count++;
    return stored;
}

template<typename T, i32 capcity>
T FixedQueue<T, capcity>::Dequeue() {
    AssertMsg(count > 0, "FixedQueue, queue is empty");
    if (count == 0) {
        return {};
    }

    T result = data[head];
    head = (head + 1) % capcity;
    count--;
    return result;
}

template<typename T, i32 capcity>
inline T * FixedQueue<T, capcity>::Peek() {
    AssertMsg(count > 0, "FixedQueue, queue is empty");
    if (count == 0) {
        return nullptr;
    }

    return &data[head];
}

template<typename T, i32 capcity>
bool FixedQueue<T, capcity>::Contains(const T & value) {
    i32 index = head;
    for (i32 i = 0; i < count; i++) {
        if (data[index] == value) {
            return true;
        }
        index = (index + 1) % capcity;
    }

    return false;
}

template<i32 capacity>
class ByteBuffer {
public:

    void CreateFromByte(u8 * bytes, u32 size) {
        AssertMsg(size < capacity, "Not enough space");
        memcpy(data, bytes, size);
    }

    template<typename _type_>
    void Push(const _type_ & value) {
        AssertMsg(head + sizeof(_type_) <= capacity, "Not enough space in buffer to push a value");
        memcpy(&data[head], &value, sizeof(_type_));
        head += sizeof(_type_);
    }

    void Push(const void * data, i32 size) {
        AssertMsg(head + size <= capacity, "Not enough space in buffer to push data");
        memcpy(&this->data[head], data, size);
        head += size;
    }

    void Push(const char * str) {
        i32 len = strlen(str);
        Push(str, len + 1); // +1 to include the null terminator
    }

    template<typename _type_, i32 cap>
    void Push(const FixedList<_type_, cap> & list) {
        Push(list.GetCount());
        // @TODO: This is slow!!
        for (i32 i = 0; i < list.GetCount(); i++) {
            Push(list[i]);
        }
    }

    i32 GetHeadLocation() {
        return head;
    }

    void HeadToStart() {
        head = 0;
    }

    template<typename _type_>
    void Yoink(_type_ & value) {
        AssertMsg(head + sizeof(_type_) <= capacity, "Not enough space in buffer to yoink a value");
        std::memcpy(&value, &data[head], sizeof(_type_));
        head += sizeof(_type_);
    }

    void Yoink(void * data, i32 size) {
        AssertMsg(head + size <= capacity, "Not enough space in buffer to yoink data");
        std::memcpy(data, &this->data[head], size);
        head += size;
    }

    void Yoink(char * str) {
        i32 len = std::strlen(str);
        Yoink(str, len + 1); // +1 to include the null terminator
    }

    u8 * GetData() {
        return data;
    }

private:
    i32 head;
    u8  data[capacity];
};
