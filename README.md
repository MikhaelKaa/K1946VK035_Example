# K1946VK035_Example
Пример проекта для микроконтроллера К1946ВК035 (К1921ВК035)

# Описание 

"under constructions"

# Сборка из изходного кода для пользователей Ubuntu (Linux)

Установите OpenOCD

https://github.com/DCVostok/openocd-k1921vk

Склонируйте репозитарий:

git clone https://github.com/MikhaelKaa/K1946VK035_Example.git

git submodule init

git submodule update

Выполните в директории репозитария:

sudo ./configure

make init

make build

cd FW

make program # Прошиква через адаптер cmsis-dap
