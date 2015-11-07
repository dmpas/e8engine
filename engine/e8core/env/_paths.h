#ifndef E8_CORE_ENV_PATHS
#define E8_CORE_ENV_PATHS

/*! Выделяет родительский каталог из пути */
void extract_directory(
                    const char     *path, /*!< Путь к файлу */
                          char     *dir   /*!< Приёмник для передачи выделеннаго каталога */
        );

/*! Приводит путь к каноническому виду */
void normalize_path(char *path);

/*! Добавляет к основанию относительный путь */
void add_path(
                          char     *path,       /*!< Каталог-основа */
                    const char     *addition    /*!< Дополнение к основе */
        );


#endif // E8_CORE_ENV_PATHS
