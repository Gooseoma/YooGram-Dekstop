# Сборка YooGram для Windows и установщик

## Способ 1. На GitHub Actions (проще всего)

Нужен публичный репозиторий (там Actions бесплатны) и **публичный `Gooseoma/lib_ui`** — на него ссылается подмодуль `Telegram/lib_ui`, иначе checkout не сможет его скачать.

1. В репозитории: **Settings → Secrets and variables → Actions → New repository secret**, добавьте
   `API_ID` и `API_HASH` — ваши данные с https://my.telegram.org (раздел API development tools).
   Без них соберётся с ограниченными тестовыми данными: войти можно, но потом начнутся ошибки.
2. **Actions → «YooGram Windows installer.» → Run workflow** (ветка `main`).
3. Первая сборка идёт 1,5–3 часа (собирает Qt и библиотеки), следующие быстрее благодаря кэшу.
4. По завершении откройте запуск → **Artifacts → YooGram-windows-x64**:
   - `YooGram-setup-x64-<версия>.exe` — установщик (Inno Setup);
   - `YooGram-portable-x64-<версия>.zip` — переносная версия (данные лежат рядом с программой).

Автообновление отключено (`DESKTOP_APP_DISABLE_AUTOUPDATE=ON`), чтобы мод не заменился официальным клиентом. Установщик и exe не подписаны — Windows SmartScreen покажет предупреждение, это нормально для неподписанных сборок.

## Способ 2. Локально

Подробно: [building-win.md](building-win.md). Коротко:

1. Visual Studio 2026 (C++ workload, SDK 10.0.26100.0), Python 3.10, Git, CMake.
2. В «x64 Native Tools Command Prompt» (с `-vcvars_ver=14.44`):

       git clone --recursive https://github.com/Gooseoma/YooGram-Desktop.git
       YooGram-Desktop\Telegram\build\prepare\win.bat
       cd YooGram-Desktop\Telegram
       configure.bat x64 qt6 -D TDESKTOP_API_ID=ВАШ_ID -D TDESKTOP_API_HASH=ВАШ_HASH -D DESKTOP_APP_DISABLE_AUTOUPDATE=ON
       cmake --build ..\out --config Release --parallel

3. Готовый `Telegram.exe` будет в `out\Release`.

### Установщик локально

1. Установите [Inno Setup 6](https://jrsoftware.org/isdl.php).
2. Скопируйте `d3dcompiler_47.dll` из `C:\Windows\System32` в `out\Release\modules\x64\d3d\`.
3. В копии `Telegram\build\setup.iss` удалите строки `SignTool=sha256` и `Source: ...Updater.exe...` (подписи и Updater у мода нет).
4. Выполните (подставьте версию из `Telegram\build\version`, например `7.2.10`):

       "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" /dMyAppVersion=7.2.10 /dMyAppVersionFull=7.2.10 "/dReleasePath=C:\путь\YooGram-Desktop\out\Release" /dMyBuildTarget=win64 /dMyOutputBaseFilename=YooGram-setup-x64-7.2.10 setup.iss

   Установщик появится в `out\Release`.

## Про старые workflow

Файлы `win.yml`, `mac.yml`, `linux.yml` и др. достались от оригинального репозитория и на каждый push/PR запускают огромные матрицы сборок (часть — на платных runner'ах `depot-*`, которых у форка нет). Для мода они не нужны: можно отключить их в **Actions → (workflow) → ⋯ → Disable workflow** или удалить файлы.
