# =======================================================
# Makefile لمشروع HPM
#
# الأهداف: all, clean, driver, setup
# =======================================================

CC = gcc
CFLAGS = -Wall -Wextra
TARGET_DRIVER = hpm_driver
SOURCE_DRIVER = hpm_file_driver.c

# الهدف الرئيسي: تجميع كل شيء (المشغل C)
all: $(TARGET_DRIVER)

# الهدف: تجميع مشغل C (hpm_driver)
$(TARGET_DRIVER): $(SOURCE_DRIVER)
	$(CC) $(CFLAGS) -o $(TARGET_DRIVER) $(SOURCE_DRIVER)
	@echo "✅ C Driver ($(TARGET_DRIVER)) built successfully."

# الهدف: إعداد الصلاحيات للسكريبتات
setup: $(TARGET_DRIVER)
	chmod +x hpm
	chmod +x start_repo.sh
	@echo "✅ Permissions set for hpm and start_repo.sh."

# الهدف: تنظيف الملفات الثنائية المُجمَّعة فقط
clean:
	rm -f $(TARGET_DRIVER)
	@echo "🧹 Cleaned built files: $(TARGET_DRIVER)"

.PHONY: all clean setup

