#!/bin/bash

# =======================================================
# GitHub Setup Script (SSH Key Management and Push)
# =======================================================

REPO_URL_HTTPS="https://github.com/adembenarfa175-code/hpm"
REPO_URL_SSH="git@github.com:adembenarfa175-code/hpm.git"
SSH_KEY_DIR="$HOME/.ssh"
SSH_KEY_FILE="$SSH_KEY_DIR/hpm_github_key" # اسم ملف مفتاح موحد

# -------------------------------------------------------
# 1. تحليل المتغيرات (Handling Arguments)
# -------------------------------------------------------

if [[ "$1" == "--nossh" ]]; then
    echo "🔑 Skipping SSH Key generation. Will attempt to use existing key."
    SKIP_SSH_GEN=true
else
    SKIP_SSH_GEN=false
fi

# -------------------------------------------------------
# 2. إنشاء مفتاح SSH جديد (Generate SSH Key)
# -------------------------------------------------------

if ! $SKIP_SSH_GEN; then
    echo "--- 1. Creating SSH Key ---"

    # إنشاء مجلد .ssh إذا لم يكن موجوداً
    mkdir -p "$SSH_KEY_DIR"
    
    # التحقق مما إذا كان المفتاح موجوداً بالفعل
    if [ -f "$SSH_KEY_FILE" ]; then
        echo "⚠️ SSH Key already exists at $SSH_KEY_FILE. Skipping generation."
    else
        # إنشاء مفتاح جديد (بواسطة خوارزمية Ed25519 الموصى بها)
        # سيطلب منك إدخال كلمة مرور (Passphrase)
        ssh-keygen -t ed25519 -f "$SSH_KEY_FILE" -C "adembenarfa175-code/hpm"
        if [ $? -ne 0 ]; then
            echo "❌ Failed to generate SSH key. Exiting."
            exit 1
        fi
        
        echo "✅ SSH Key generated successfully at $SSH_KEY_FILE."
        echo "🚨 Remember to ADD the public key ($SSH_KEY_FILE.pub) to your GitHub account settings!"
    fi
fi

# -------------------------------------------------------
# 3. فتح مفتاح SSH (Add Key to ssh-agent)
# -------------------------------------------------------

echo "--- 2. Starting ssh-agent and adding key ---"

# بدء وكيل SSH (ssh-agent) إذا لم يكن يعمل
eval "$(ssh-agent -s)"

# إضافة المفتاح الخاص لوكيل SSH (سيطلب كلمة المرور إذا تم تعيين واحدة)
ssh-add "$SSH_KEY_FILE"

# -------------------------------------------------------
# 4. الاتصال بالمستودع والرفع (Connect and Push)
# -------------------------------------------------------

echo "--- 3. Connecting to Repository and Pushing ---"

# التأكد من ربط المستودع بالبروتوكول الآمن (SSH)
if git remote get-url origin &> /dev/null; then
    echo "🔗 Remote 'origin' already exists. Updating URL to SSH."
    git remote set-url origin "$REPO_URL_SSH"
else
    echo "🔗 Remote 'origin' does not exist. Adding SSH URL."
    git remote add origin "$REPO_URL_SSH"
fi

# دفع الكود إلى الفرع الرئيسي (main)
echo "🚀 Pushing code to GitHub..."
git push -u origin main

if [ $? -eq 0 ]; then
    echo "✅ Success: Code pushed to $REPO_URL_SSH"
else
    echo "❌ Error: Git push failed. Ensure your public key is added to GitHub."
fi

