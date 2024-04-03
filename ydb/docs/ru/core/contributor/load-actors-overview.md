# Обзор тестирования с помощью нагружающих акторов

Тестирование производительности системы является важным этапом добавления изменений в ядро {{ ydb-short-name }}. С помощью [нагрузочного тестирования](https://ru.wikipedia.org/wiki/Нагрузочное_тестирование) вы можете:

* определить показатели производительности и сравнить их со значениями до изменений;
* проверить работоспособность системы на высоких или пиковых нагрузках.

Сама по себе задача нагрузить высокопроизводительную распределенную систему является нетривиальной. Часто разработчику приходится запускать несколько инстансов клиента, чтобы добиться требуемой загрузки сервера. {{ ydb-short-name }} реализует простой и удобный механизм подачи нагрузки — нагружающие акторы. Акторы создаются и запускаются в самом кластере, таким образом не нужно привлекать дополнительные ресурсы для запуска клиентов. Нагружающие акторы могут быть запущены на произвольных узлах кластера — на одном, на всех или только на выбранных. Вы можете создать любое количество акторов на любом из узлов.

Нагружающие акторы позволяют тестировать как систему в целом, так отдельные ее компоненты:

<center>

![load-actors](../_assets/load-actors.svg)

</center>

Например, вы можете подать [нагрузку на Distributed Storage](load-actors-storage.md) без задействования слоев таблеток и Query Processor. Таким образом можно изолированно тестировать отдельные слои системы и эффективно находить узкие места. Комбинация акторов разных типов позволяет запускать множество видов нагрузки.

{% include [release-candidate](../_includes/trunk.md) %}

## Типы акторов {#load-actor-type}

Тип | Описание
--- | ---
[KqpLoad](load-actors-kqp.md) | Подает нагрузку на слой Query Processor и нагружает все компоненты кластера.
[KeyValueLoad](load-actors-key-value.md) | Нагружает Key-value таблетку.
[StorageLoad](load-actors-storage.md) | Нагружает Distributed Storage без задействования слоев таблеток и Query Processor.
[VDiskLoad](load-actors-vdisk.md) | Тестирует производительность записи на VDisk.
[PDiskWriteLoad](load-actors-pdisk-write.md) | Тестирует производительность записи на PDisk.
[PDiskReadLoad](load-actors-pdisk-read.md) | Тестирует производительность чтения с PDisk.
[PDiskLogLoad](load-actors-pdisk-log.md) | Тестирует корректность вырезания из середины лога PDisk.
[MemoryLoad](load-actors-memory.md) | Аллоцирует память, полезен при тестировании логики.
[Stop](load-actors-stop.md) | Останавливает все акторы, либо только указанные.

## Запуск нагрузки {#load-actor-start}

Запустить нагрузку можно с помощью следующих инструментов:

* Embedded UI кластера — позволяет создать по конфигурации и запустить нагружающий актор либо на текущем узле, либо сразу на всех узлах тенанта.
* Утилита `ydbd` — позволяет на любой узел кластера отправить конфигурацию актора с указанием, на каких узлах необходимо создать и запустить актор.

В качестве примера рассмотрим создание и запуск актора KqpLoad. Актор обращается к БД `/slice/db` как к key-value хранилищу в 64 потока, длительность нагрузки — 30 секунд. Перед началом работы актор создает необходимые таблицы, после окончания удаляет их. При создании актору будет автоматически присвоен тег. Этот же тег будет присвоен и результату теста.

{% list tabs %}

- Embedded UI

  1. Откройте страницу управления нагружающими акторами на узле (например, `http://<address>:8765/actors/load`, где `address` — адрес узла кластера, на котором нужно запустить нагрузку).
  1. В поле ввода/вывода вставьте конфигурацию актора:

      ```proto
      KqpLoad: {
          DurationSeconds: 30
          WindowDuration: 1
          WorkingDir: "/slice/db"
          NumOfSessions: 64
          UniformPartitionsCount: 1000
          DeleteTableOnFinish: 1
          WorkloadType: 0
          Kv: {
              InitRowCount: 1000
              PartitionsByLoad: true
              MaxFirstKey: 18446744073709551615
              StringLen: 8
              ColumnsCnt: 2
              RowsCnt: 1
          }
      }
      ```

  1. Чтобы создать и запустить актор, нажмите кнопку:
      * **Start new load on current node** — нагрузка будет запущена на текущем узле.
      * **Start new load on all tenant nodes** — нагрузка будет запущена на всех узлах тенанта.
  
  В поле ввода/вывода появится следующее сообщение:

  ```text
  {"status":"OK","tag":1}
  ```

  * `status` — статус запуска нагрузки;
  * `tag` — тег, который был присвоен нагрузке.

- CLI

  1. Создайте файл с конфигурацией актора:

      ```proto
      NodeId: 1
      Event: {
          KqpLoad: {
              DurationSeconds: 30
              WindowDuration: 1
              WorkingDir: "/slice/db"
              NumOfSessions: 64
              UniformPartitionsCount: 1000
              DeleteTableOnFinish: 1
              WorkloadType: 0
              Kv: {
                  InitRowCount: 1000
                  PartitionsByLoad: true
                  MaxFirstKey: 18446744073709551615
                  StringLen: 8
                  ColumnsCnt: 2
                  RowsCnt: 1
              }
          }
      }
      ```

      * `NodeId` — идентификатор узла, на котором нужно запустить актор. Чтобы указать несколько узлов, перечислите их в отдельных строках:

        ```proto
        NodeId: 1
        NodeId: 2
        ...
        NodeId: N
        Event: {
        ...
        ```

      * `Event` — конфигурация актора.

  1. Запустите актор:

      ```bash
      ydbd load-test --server <endpoint> --protobuf "$(cat <proto_file>)"
      ```

      * `endpoint` — grpc-эндпоит узла (например, `grpc://<address>:<port>`, где `address` — адрес узла, `port` — grpc-порт узла).
      * `proto_file` — путь к файлу с конфигурацией актора.

{% endlist %}

## Просмотр результата тестирования {#view-results}

Результаты тестирования можно просмотреть с помощью Embedded UI. Описание выводимых параметров смотрите в документации соответствующего актора.

{% list tabs %}

- Embedded UI

  1. Откройте страницу управления нагружающими акторами на узле (например, `http://<address>:<port>/actors/load`, где `address` — адрес узла, `port` — http-порт мониторинга узла, на котором была запущена нагрузка).
  1. Нажмите кнопку **Results**.

      Будут отображены результаты завершенных тестирований. Найдите результат с соответствующим тегом.

      ![load-actors-finished-tests](../_assets/load-actors-finished-tests.png)

{% endlist %}