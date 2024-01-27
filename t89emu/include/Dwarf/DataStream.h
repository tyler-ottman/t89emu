#pragma once

#include <cstddef>
#include <cstdint>

class DataStream {
public:
    DataStream(const uint8_t *data, size_t streamLen);
    DataStream(const uint8_t *data);
    ~DataStream();

    // For reading data from byteStream
    uint8_t decodeUInt8(void);
    uint16_t decodeUInt16(void);
    uint32_t decodeUInt32(void);
    int64_t decodeLeb128(void);
    size_t decodeULeb128(void);

    bool isStreamable(void);
    const uint8_t *getData(void);
    const uint8_t *getData(size_t idx);
    size_t getIndex(void);

    void setData(const uint8_t *data);
    void setOffset(size_t offset); // Offset from center
    void setLen(size_t len);

private:
    const uint8_t *data;
    size_t index;
    size_t streamLen;
};