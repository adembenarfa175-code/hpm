#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // لأجل mkdir
#include <errno.h>    // لأجل errno (لم يتم استخدامه مباشرة هنا، لكنه مفيد)

// -------------------------------------------------------------------
// (1) تصريح بوظائف ملف hpm_file_driver.c
// -------------------------------------------------------------------

// وظائف التوجيه من ملف hpm_file_driver.c
int hpm_unpack(const char* package_path, const char* extract_path);
int hpm_execute_c_install(const char* package_name, const char* extract_path);
void hpm_cleanup(const char* package_name); // وظيفة لتنظيف الملفات المؤقتة

// -------------------------------------------------------------------
// (2) التعريفات الثابتة والمسارات
// -------------------------------------------------------------------

// العنوان الأصلي الذي يعمل محلياً دون تعديل hosts
#define REPO_URL_BASE "http://localhost:8080/" 
#define DOWNLOAD_DIR "/tmp/hpm_downloads/"
#define EXTRACT_DIR "/tmp/hpm_build/"

// -------------------------------------------------------------------
// (3) وظيفة البحث (Search and Locate)
// -------------------------------------------------------------------

/**
 * تبحث عن رابط الحزمة في المستودعات. 
 * @param package_name اسم الحزمة.
 * @return رابط الحزمة الكامل (.hpm) إذا وُجدت، أو NULL.
 */
char* search_repository(const char* package_name) {
    
    // بناء الرابط: http://localhost:8080/PackageName.hpm
    size_t url_len = strlen(REPO_URL_BASE) + strlen(package_name) + strlen(".hpm") + 1;
    char* full_url = (char*)malloc(url_len);

    if (full_url == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    snprintf(full_url, url_len, "%s%s.hpm", REPO_URL_BASE, package_name);

    return full_url;
}

// -------------------------------------------------------------------
// (4) وظيفة التنزيل (Download) - تمت إضافة إنشاء الملف الفعلي مؤقتاً
// -------------------------------------------------------------------

/**
 * تنفذ عملية تنزيل الملف الثنائي (محاكاة).
 * *ملاحظة*: تم تعديل هذه الوظيفة لإنشاء ملف التنزيل فعلياً لكي يعمل tar.
 * @param package_url رابط الحزمة.
 * @param local_path المسار المحلي لحفظ الحزمة.
 * @return 0 للنجاح، ورمز خطأ آخر.
 */
int download_package(const char* package_url, const char* local_path) {
    // إنشاء مجلد التنزيل إذا لم يكن موجوداً
    mkdir(DOWNLOAD_DIR, 0755); 

    printf("[hpm:1] %s\n", package_url);
    printf("Network           Package\n");

    // محاكاة شريط التقدم 
    printf("**100%%            [####################]\n");

    // **************** التعديل الحرج للتشغيل ****************
    // تنفيذ أمر Shell لنسخ الملف الذي تم تنزيله من المستودع التجريبي
    // نفترض أن المستودع قيد التشغيل ووصل إليه (بسبب curl/wget في الخلفية).
    char command[512];
    // استخدام wget أو curl لتنزيل الملف إلى المسار المحلي
    snprintf(command, sizeof(command), "wget -q -O %s %s", local_path, package_url);
    
    // في بيئة جذر، wget هو خيار جيد للتنزيل دون الحاجة لربط libcurl
    int status = system(command); 
    // **************** نهاية التعديل الحرج ****************

    if (status == 0) {
        printf("[hpm:2] OK download %s\n", local_path);
        return 0;
    } else {
        fprintf(stderr, "Error: Download failed for %s. Status: %d\n", package_url, status);
        return 1;
    }
}

// -------------------------------------------------------------------
// (5) الوظيفة الرئيسية: نقطة الدخول لـ hpm_core
// -------------------------------------------------------------------

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: No command provided. Run 'hpm help'.\n");
        return 1;
    }
    
    // توجيه الأوامر (Command Dispatcher)
    if (strcmp(argv[1], "install") == 0) {
        // أمر 'hpm install <package>'
        if (argc < 3) {
            fprintf(stderr, "Error: Package name required for 'install'.\n");
            return 1;
        }
        
        const char* package_name = argv[2];
        char* package_url;
        char local_path[512];
        char extract_path[512];

        printf("HPM C-Core: Starting installation workflow for: %s\n", package_name);

        // 1. البحث
        package_url = search_repository(package_name);
        if (package_url == NULL) {
            fprintf(stderr, "Error: Could not locate package '%s'.\n", package_name);
            return 1;
        }

        // 2. تحديد مسار التنزيل والتنزيل الفعلي
        snprintf(local_path, sizeof(local_path), "%s%s.hpm", DOWNLOAD_DIR, package_name);
        if (download_package(package_url, local_path) != 0) {
            goto cleanup_failure;
        }
        
        // 3. تحديد مسار فك الضغط
        snprintf(extract_path, sizeof(extract_path), "%s%s/", EXTRACT_DIR, package_name);
        
        // 4. فك الضغط (استدعاء hpm_file_driver.c)
        if (hpm_unpack(local_path, EXTRACT_DIR) != 0) { 
            goto cleanup_failure;
        }

        // 5. التجميع والتثبيت (استدعاء hpm_file_driver.c)
        if (hpm_execute_c_install(package_name, EXTRACT_DIR) != 0) {
            goto cleanup_failure;
        }
        
        // 6. التنظيف والانتهاء بنجاح
        hpm_cleanup(package_name);
        printf("[HPM] Package '%s' installed successfully.\n", package_name);

        free(package_url);
        return 0;

        // في حال فشل أي خطوة بعد التنزيل
        cleanup_failure:
            fprintf(stderr, "Installation of '%s' failed. Attempting cleanup...\n", package_name);
            hpm_cleanup(package_name);
            free(package_url);
            return 1;

    } else if (strcmp(argv[1], "update_system") == 0) {
        // تنفيذ أمر 'hpm update'
        printf("HPM C-Core: Running system update logic...\n");
        return 0;

    } else if (strcmp(argv[1], "search_repo") == 0) {
        // تنفيذ أمر 'hpm search <query>'
        printf("HPM C-Core: Searching repository for '%s'...\n", argv[2]);
        return 0;

    } else {
        // أمر غير معروف
        fprintf(stderr, "Error: Unknown command passed to hpm_core.\n");
        return 1;
    }
}

