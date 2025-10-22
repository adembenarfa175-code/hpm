#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // لأجل mkdir
#include <unistd.h>   // لأجل rmdir/unlink
#include <errno.h>    // <-- مُضاف لحل خطأ errno و EEXIST

// المسار حيث تم تنزيل الحزمة (موجود أيضاً في hpm_core.c)
#define DOWNLOAD_DIR "/tmp/hpm_downloads/"
// المسار المؤقت لفك ضغط الحزمة
#define EXTRACT_DIR "/tmp/hpm_build/"

// -------------------------------------------------------------------
// وظيفة 1: فك ضغط ملف الحزمة
// -------------------------------------------------------------------

/**
 * يفك ضغط ملف .hpm (المفترض أنه أرشيف tarball) إلى مسار مؤقت.
 * @param package_path المسار الكامل لملف الحزمة .hpm المنزّل.
 * @param extract_base_path المسار الأساسي لفك الضغط (/tmp/hpm_build/).
 * @return 0 للنجاح، ورمز خطأ آخر.
 */
int hpm_unpack(const char* package_path, const char* extract_base_path) {
    // إنشاء مسار التثبيت المؤقت. نتحقق من EEXIST لتجنب الفشل إذا كان المجلد موجوداً.
    if (mkdir(extract_base_path, 0755) != 0 && errno != EEXIST) {
        perror("[HPM Driver] Error creating extraction directory");
        return 1;
    }

    printf("[HPM Driver] Unpacking package...\n");

    // تنفيذ أمر Shell لفك الضغط
    char command[512];
    snprintf(command, sizeof(command), "tar -xf %s -C %s", package_path, extract_base_path);

    int status = system(command);

    if (status == 0) {
        printf("[HPM Driver] OK Unpacked successfully to %s\n", extract_base_path);
        return 0;
    } else {
        fprintf(stderr, "[HPM Driver] Error: Failed to unpack package %s. Status: %d\n", package_path, status);
        return 1;
    }
}

// -------------------------------------------------------------------
// وظيفة 2: تجميع وتنفيذ سكربت التثبيت الداخلي المكتوب بلغة C
// -------------------------------------------------------------------

/**
 * يجد install_script.c داخل الحزمة ويقوم بتجميعه وتنفيذه.
 * @param package_name اسم الحزمة.
 * @param extract_base_path المسار الذي يحتوي على محتويات الحزمة.
 * @return 0 للنجاح، ورمز خطأ آخر.
 */
int hpm_execute_c_install(const char* package_name, const char* extract_base_path) {
    char source_file[512];
    char executable_file[512];
    
    // بناء المسارات: /tmp/hpm_build/PackageName/install_script.c
    snprintf(source_file, sizeof(source_file), "%s%s/install_script.c", extract_base_path, package_name);
    snprintf(executable_file, sizeof(executable_file), "%s%s/hpm_installer_temp", extract_base_path, package_name);

    // التحقق من وجود ملف سكربت التثبيت
    if (access(source_file, F_OK) != 0) {
        fprintf(stderr, "[HPM Driver] Error: install_script.c not found in the package.\n");
        return 1;
    }

    printf("[HPM Driver] Compiling embedded C installer...\n");

    // 1. التجميع (Compilation): استخدام GCC لتجميع install_script.c
    // تم زيادة الحجم إلى 2048 لحل format-truncation
    char compile_command[2048]; 
    
    snprintf(compile_command, sizeof(compile_command), "gcc %s -o %s", source_file, executable_file);
    
    if (system(compile_command) != 0) {
        fprintf(stderr, "[HPM Driver] Error: Failed to compile the package's C install script.\n");
        return 1;
    }

    // 2. التنفيذ (Execution): تشغيل الملف التنفيذي الذي تم تجميعه للتو
    printf("[HPM Driver] Running C installation script...\n");

    // محاكاة شريط التقدم 
    printf("Compiling\n");
    printf("## Code ## [#####                 ] 25%%\n");
    
    // تنفيذ سكربت التثبيت الفعلي
    int install_status = system(executable_file);

    if (install_status == 0) {
        printf("## Code ## [####################] 100%%\n");
        printf("[HPM Driver] C install script executed successfully.\n");
        return 0;
    } else {
        fprintf(stderr, "[HPM Driver] Error: Package C installation script failed. Status: %d\n", install_status);
        return 1;
    }
}

// -------------------------------------------------------------------
// وظيفة 3: التنظيف بعد التثبيت (Cleanup)
// -------------------------------------------------------------------

/**
 * تنظيف الملفات المؤقتة بعد التثبيت (ملف .hpm المنزّل ومجلد البناء).
 * @param package_name اسم الحزمة.
 */
void hpm_cleanup(const char* package_name) {
    char download_path[512];
    char extract_root[512];
    
    // 1. تنظيف ملف .hpm المنزّل: /tmp/hpm_downloads/PackageName.hpm
    snprintf(download_path, sizeof(download_path), "%s%s.hpm", DOWNLOAD_DIR, package_name);
    if (unlink(download_path) == 0) {
        printf("[HPM Cleanup] Removed downloaded package: %s\n", download_path);
    } else {
        // رسالة تحذيرية إذا فشل الحذف
    }

    // 2. تنظيف مجلد فك الضغط والتجميع: /tmp/hpm_build/
    snprintf(extract_root, sizeof(extract_root), "%s", EXTRACT_DIR);
    
    // تم زيادة الحجم إلى 1024 لحل format-truncation في هذه الدالة
    char rmdir_command[1024]; 
    snprintf(rmdir_command, sizeof(rmdir_command), "rm -rf %s", extract_root);
    
    if (system(rmdir_command) == 0) {
        printf("[HPM Cleanup] Removed extraction directory: %s\n", extract_root);
    } else {
        fprintf(stderr, "[HPM Cleanup] Warning: Could not remove extraction directory.\n");
    }
}

