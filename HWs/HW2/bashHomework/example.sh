#!/bin/bash
echo "сегодня " `date`

echo -e "\n введите путь к директории:"
read path_to

echo -e "\n ваша директория содержит файлы и папки:"
ls $path_to
