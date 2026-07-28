# hazm_cpp

پورت حداقلی از کتابخانه **hazm** پایتون به زبان **C++17**

این پروژه یک پورت کامل و صد درصد سازگار از بخش‌های مورد نیاز کتابخانه hazm برای استفاده در پروژه
**[Piper-with-LCA-Phonemizer](https://github.com/user/Piper-with-LCA-Phonemizer)** می‌باشد.

فایل `correct_phonemes.py` در Piper تنها از دو بخش hazm استفاده می‌کند:

- **Lemmatizer** (ریشه‌یاب)
- **stopwords_list** (کلمات ایست فارسی)

## بخش‌های پیاده‌سازی شده

| کامپوننت | توضیح | وضعیت |
|---|---|---|
| **Stemmer** | ریشه‌یاب قاعده‌مند (حذف پسوندها) | ✅ |
| **Conjugation** | صرف فعل فارسی (بیش از ۵۰ زمان) | ✅ |
| **WordTokenizer** | بارگذاری دیکشنری کلمات و افعال | ✅ (فقط بارگذاری داده) |
| **Lemmatizer** | ریشه‌یاب مبتنی بر دیکشنری | ✅ |
| **Stopwords** | لیست ۳۸۹ کلمه ایست فارسی | ✅ |

## پیش‌نیازها

- کامپایلر ++C با پشتیبانی از C++17
- CMake نسخه ۳.۱۴ یا بالاتر
- Catch2 (برای تست‌ها — به صورت خودکار توسط CMake دانلود می‌شود)

## روش کامپایل و اجرای تست‌ها

```bash
cd hazm_cpp
mkdir build && cd build
cmake .. -DBUILD_TESTS=ON
make -j$(nproc)
./hazm_tests
```

## خروجی تست‌ها

```
All tests passed (40 assertions in 15 test cases)
```

## وابستگی‌ها

این پروژه **هیچ وابستگی خارجی** ندارد. تمامی فایل‌های دیتاست (`words.dat`، `verbs.dat`، `stopwords.dat`) از مخزن اصلی hazm کپی شده‌اند.

## معماری

```
hazm_cpp/
├── CMakeLists.txt
├── data/
│   ├── words.dat          # دیکشنری ۲.۲ مگابایتی کلمات فارسی
│   ├── verbs.dat          # ۶۹۳ ریشه فعل
│   └── stopwords.dat      # ۳۸۹ کلمه ایست
├── include/hazm/
│   ├── hazm.hpp           # هدر اصلی
│   ├── constants.hpp      # جداول یونیکد، پسوندها
│   ├── utils.hpp          # بارگذاری فایل‌ها، تبدیل UTF-8
│   ├── stemmer.hpp        # Stemmer
│   ├── conjugation.hpp    # صرف فعل
│   ├── word_tokenizer.hpp # بارگذاری داده‌ها
│   ├── lemmatizer.hpp     # Lemmatizer
│   └── stopwords.hpp      # کلمات ایست
├── src/                   # پیاده‌سازی
└── tests/                 # تست‌ها (Catch2)
```

## سازگاری با hazm پایتون

تمام خروجی‌ها با نسخه پایتون hazm مقایسه و تأیید شده‌اند. تست‌های cross-validate تطابق ۱۰۰٪ را تضمین می‌کنند.

## لایسنس

این پروژه تحت همان لایسنس کتابخانه اصلی hazm منتشر می‌شود.