# rbuilder

Приложение для удаленной сборки с/с++ проектов под ОС Linux.

В качестве транспорта используется ssh. В следствии чего перед использованием софта необходимо настроить
доступ по ключам к целевому серверу

Для манипулирования удаленной системой и сборкой используются комманды терминала. Для каждой из предусмотренных ролей 
(build, install, clean) в конфигурации имеется соответствующий массив для перечня этих комманд.

Для установки зависимостей необходимо настроить права учетной записи под которой происходит соединение 
с удаленной системой

Использование:
- Создаем конфигурационный файл rbuilder.conf в папке с проектом который необходимо собрать на удаленном хосте.
- В нем указываем рабочую папку в которую будут перенесены все файлы и каталоги из текущей директории.
- Далее запускаем в папке с проктом для сборки rbuilder build | install | clean.
- В stdout выводятся этапы сборки. Вывод всех ошибок идет туда же.