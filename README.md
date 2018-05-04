# meta-rnox Yocto Project layer for R-NOX project

Основным мануалом является следующий ресурс: [ConnectCore 6UL SBC Express Documentation](https://www.digi.com/resources/documentation/digidocs/90002286/#landing_pages/cc6ul_index.htm%3FTocPath%3D_____1).

1. Установка **Digi Embedded Yocto** осуществляется в соответствии со следующей инструкцией - [Install Digi Embedded Yocto](https://www.digi.com/resources/documentation/digidocs/90002286/#task/eclipse_ide/new/install_digi_embedded_yocto.htm%3FTocPath%3DDigi%2520Embedded%2520Yocto%7CSystem%2520development%7C_____2).

* Устанавливается **repo** tool

```
    sudo curl -o /usr/local/bin/repo http://commondatastorage.googleapis.com/git-repo-downloads/repo
    sudo chmod a+x /usr/local/bin/repo
```

* Создаем папку для установки с разрешением на запись (чтобы узнать пользователя и группу можно использовать команду **id**):

```    
    sudo install -o <your-user> -g <your-group> -d /your/path/dey-2.4
    cd /your/path/dey-2.4
```

* Скачиваем репозиторий:

```    
    repo init -u https://github.com/digi-embedded/dey-manifest.git -b rocko
    repo sync -j8 --no-repo-verify
```

2. Создание проекта производится по инструкции - [Create and build projects](https://www.digi.com/resources/documentation/digidocs/90002286/#task/yocto/t_create_build_projects_yocto.htm%3FTocPath%3DDigi%2520Embedded%2520Yocto%7CSystem%2520development%7C_____3).

    * Инициализация проекта и среды окружения:

```    
    mkdir -p ${HOME}/workspace/ccimx6ulstarter
    cd ${HOME}/workspace/ccimx6ulstarter
    source /your/path/dey-2.4/mkproject.sh -p ccimx6ulstarter
```

3. Скачиваем наш **meta-rnox layer** в директорию с Yocto:

```
    cd /your/path/dey-2.4/sources
    git clone https://github.com/R-NOX/meta-rnox
```

4. Добавляем наш layer в конфиг проекта **${HOME}/workspace/ccimx6ulstarter/conf/bblayers.conf**:

```
    BBLAYERS ?= " \
  /your/path/dey-2.4/sources/poky/meta \
  ...
  ...
  /your/path/dey-2.4/sources/meta-digi/meta-digi-arm \
  /your/path/dey-2.4/sources/meta-digi/meta-digi-dey \
  **/your/path/dey-2.4/sources/meta-rnox** \
  "
```

5. Для сборки проекта нужно выполнить команду `bitbake image-rnox`. Если не распознает команду `bitbake`, то следует сконфигурировать окружение при помощи скрипта **dey-setup-environment**, который лежит в папке проекта (смотри пункт 2). Это нужно проделывать каждый раз, когда вы открываете новое окно терминала или новую вкладку в терминале. Если при сборке image-rnox не собирается какой-то отдельный пакет, попробуйте собрать его отдельно при помощи команды `bitbake package-name` и потом снова запустить `bitbake image-rnox`.

6. После сборки нужные файлы будут находится в директории **${HOME}/workspace/ccimx6ulstarter/tmp/deploy/images/ccimx6ulstarter**. Из этой директории на microSD надо скопировать следующие файлы:
* image-rnox-ccimx6ulstarter-20180504154445.boot.ubifs 		-> *переименовать в* -> image-rnox-ccimx6ulstarter.boot.ubifs
* image-rnox-ccimx6ulstarter-20180504154445.recovery.ubifs 	-> *переименовать в* -> image-rnox-ccimx6ulstarter.recovery.ubifs
* image-rnox-ccimx6ulstarter-20180504154445.rootfs.ubifs 	-> *переименовать в* -> image-rnox-ccimx6ulstarter.rootfs.ubifs
* u-boot-ccimx6ulstarter-2015.04-r0.imx 			-> *переименовать в* -> u-boot-ccimx6ulstarter.imx
* install\_rnox\_fw\_sd.scr

7. Установить microSD в борду, сделать перезагрузку устройства при помощи кнопки, когда появится надпись "*Hit any key to stop autoboot:*", нажать любую клавишу на клавиатуре. После этого выполнить команду:

    `run install_rnox`

Новая сборка установится в NAND память. Больше информации можно найти [здесь](https://www.digi.com/resources/documentation/digidocs/90002286/#task/yocto/t_program_firmware_yocto.htm%3FTocPath%3DDigi%2520Embedded%2520Yocto%7CGet%2520started%7C_____3) и [здесь](https://www.digi.com/resources/documentation/digidocs/90002286/#task/yocto/t_update_fw_from_usd_yocto.htm%3FTocPath%3DDigi%2520Embedded%2520Yocto%7CSystem%2520development%7CProgram%2520devices%7CUpdate%2520firmware%7CTransfer%2520the%2520firmware%2520to%2520the%2520module%7CProgram%2520the%2520firmware%2520from%2520U-Boot%7C_____2).

