#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

class FlashStorage
{
public:
    static FlashStorage &getInstance(void);

private:
    FlashStorage();
    FlashStorage(const FlashStorage &) = delete;
    FlashStorage &operator=(const FlashStorage &) = delete;
};

#endif // FLASH_STORAGE_H