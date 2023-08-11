@echo off

where /q cl || (
  echo ERROR: "cl" not found - please run this from the MSVC x86 native tools command prompt
  exit /b 1
)

git rev-parse --short HEAD > pm_version.txt
set /p PM_VERSION=<pm_version.txt
call cl /DNDEBUG /DPM_THREAD_COUNT=32 /DPM_VERSION=%PM_VERSION% /DNDEBUG /O2 /Fe:pm-msvc.exe /nologo /Iinclude source/console/pm_main.c source/console/pm_thread.c source/console/pm_aes.c source/console/pm_b64.c source/console/pm_io.c source/console/pm_parse.c source/console/pm_rand.c source/console/pm_sync.c source/console/pm_glob.c source/console/pm_xcrypt.c
set PM_VERSION=
del pm_version.txt
