#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// اسم ملف التثبيت الذي نبحث عنه داخل الحزمة
#define INSTALL_SCRIPT_NAME "install_script.c"
// اسم البرنامج التنفيذي الذي سيتم تجميعه من ملف التثبيت
#define INSTALL_PROGRAM_NAME "hpm_installer_temp"

// =======================================================
// وظيفة تشغيل الأمر وتنفيذ التجميع
// =======================================================
int run_command(const char *command) {
    printf("[HPM Driver] Executing: %s\n", command);
    int status = system(command);
    if (status != 0) {
        // حالة الخروج هي قيمة 8-bit العلوية
        fprintf(stderr, "[HPM Driver] Error: Command failed. Status: %d\n", status);
    }
    return status;
}

// =======================================================
// الدالة الرئيسية: تتولى فك الضغط، التجميع، والتنفيذ
// =======================================================
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: ./hpm_driver <package_path> <extract_base_path>\n");
        return 1;
    }

    const char *package_path = argv[1];     // /tmp/hpm_downloads/ExamplePackage.hpm
    const char *extract_base_path = argv[2]; // /tmp/hpm_build

    // [إصلاح CVE-HPM-00000A1]: زيادة حجم المخزن المؤقت للأوامر
    char command[1024]; 
    int status;

    // 1. إنشاء مجلد البناء (الاستخراج)
    printf("[HPM Driver] Creating build directory...\n");
    if (mkdir(extract_base_path, 0777) != 0) {
        // إذا فشل الإنشاء، فقد يكون المجلد موجوداً. نتابع.
    }
    
    // 2. فك ضغط الحزمة
    printf("[HPM Driver] Unpacking package...\n");
    // نستخدم 'tar -xf' لفك ضغط أرشيف tar الخام
    snprintf(command, sizeof(command), "tar -xf %s -C %s", package_path, extract_base_path);
    status = run_command(command);

    if (status != 0) {
        fprintf(stderr, "[HPM Driver] Error: Failed to unpack package %s. Status: %d\n", package_path, status);
        return 512; // رمز خطأ مخصص
    }
    printf("[HPM Driver] OK Unpacked successfully to %s\n", extract_base_path);

    // بناء مسارات الملفات
    char install_script_path[256];
    char install_program_path[256];
    snprintf(install_script_path, sizeof(install_script_path), "%s/%s", extract_base_path, INSTALL_SCRIPT_NAME);
    snprintf(install_program_path, sizeof(install_program_path), "%s/%s", extract_base_path, INSTALL_PROGRAM_NAME);

    // 3. التجميع (Compilation)
    printf("[HPM Driver] Compiling embedded C installer...\n");
    // gcc -o /tmp/hpm_build/hpm_installer_temp /tmp/hpm_build/install_script.c
    snprintf(command, sizeof(command), "gcc -o %s %s", install_program_path, install_script_path);
    status = run_command(command);

    if (status != 0) {
        fprintf(stderr, "[HPM Driver] Error: Failed to compile C installer. Status: %d\n", status);
        return 2;
    }

    // 4. التنفيذ (Execution)
    printf("[HPM Driver] Running C installation script...\n");
    // /tmp/hpm_build/hpm_installer_temp
    snprintf(command, sizeof(command), "%s", install_program_path);
    status = run_command(command);

    if (status != 0) {
        fprintf(stderr, "[HPM Driver] Error: C install script failed execution. Status: %d\n", status);
        return 3;
    }

    printf("[HPM Driver] C install script executed successfully.\n");

    return 0;
}

