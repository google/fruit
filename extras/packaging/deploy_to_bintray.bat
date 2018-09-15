set FRUIT_VERSION=3.4.0

for %%G in (Release Debug) DO CMD /C for %%H in (True False) DO CMD /C for %%I in (True False) DO conan create . google/stable -o fruit:shared=%%H -o fruit:use_boost=%%I -s build_type=%%G

conan remote add fruit-bintray https://api.bintray.com/conan/google/fruit

REM To authenticate:
REM conan user -p BINTRAY_API_KEY_HERE -r fruit-bintray polettimarco

conan upload fruit/%FRUIT_VERSION%@google/stable --all -r fruit-bintray
