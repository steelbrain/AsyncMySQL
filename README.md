Async-MySQL
===========
An extremely lightweight, Productive wrapper for [HHVM][HHVM]'s [HackLang][HackLang].

#### The Problem
HackLang's original `query` and `queryf` implementation is great. But, it becomes really complex on large queries.
```hack
$Query = await $DBAsync->queryf("Update Users set Plan = %d, Status = %d, FirstName = %s, LastName = %s, LastActive = %d, Server = (Select ID from Servers where SecretToken = %d) where ID = %d LIMIT 1",
(int) $Processed['plan'], (int) $Processed['status'], $Processed['firstname'], $Processed['lastname'], (int) $Processed['lastactive'], (int) $Processed['servertoken'], (int) $Processed['int']);
```
Imagine how would you supply each parameter without not making a mistake or without not being confused and not spending your precious time debugging, Now that was just a simple query, I have some quite long ones in some of my projects and `AsyncMySQLConnection` got more and more confusing with them.

#### The Solution
Async-MySQL introduces your old friend Mr PDO Parameters in your Async Queries. So doing the query with Async-MySQL would be like

```hack
$Query = await $DBAsync->query("Update Users set Plan = :plan, Status = :status, FirstName = :fname, LastName = :lname, LastActive = :lactive, Server = (Select ID from Servers where SecretToken = :token) where ID = :id LIMIT 1",
   ImmMap{
    ':plan' => $Processed['plan'],
    ':status' => $Processed['status'],
    ':fname' => $Processed['firstname'],
    ':lname' => $Processed['lastname'],
    ':lactive' => $Processed['lastactive'],
    ':token' => $Processed['token'],
    ':id' => $Processed['id']
});
```

#### Examples
AsyncMySQL really makes SQL queries a piece of non-confusing straight-forward cake.
```hack
$DB = await AsyncDatabase::connect("records");
$InsertID = $DB->insertSync("Clients", Map{
  'Username' => $Info['Username'],
  'Password' => $Info['Password'],
  'Email' => $Info['Email'],
  'Confirmed' => '0'
});
var_dump($InsertID); // Ouputs a numeric value
$Exists = $DB->existsSync("Clients", Map{'ID' => $InsertID});
var_dump($Exists); // Outputs `bool(true)`
$AffectedRows = $DB->updateSync("Clients", Map{'ID' => $InsertID}, Map{'Confirmed' => '1'});
var_dump($AffectedRows); // Outputs `int(1)`
$DeletedRows = $DB->deleteSync("Clients", Map{'ID' => $InsertID});
var_dump($DeletedRows); // Outputs `int(1)`
```

#### API
```hack
type AsyncDatabaseResult = shape('Count' => int, 'Rows' => Vector<Map<string, string>>, 'ID' => int)
class AsyncDatabase{
  public static async function connect(string $Host = "localhost", string $Database, int $Port = 3306, string $User = 'root', string $Password = '', int $Timeout = -1):Awaitable<AsyncDatabase>
  public function __construct(private AsyncMysqlConnection $Con)
  public async function query(string $Query, KeyedContainer<string, string> $Arguments):Awaitable<AsyncDatabaseResult>
  public function querySync(string $Query, KeyedContainer<string, string> $Arguments):AsyncDatabaseResult
  public async function insert(string $Table, KeyedContainer<string, string> $Arguments):Awaitable<int>
  public function insertSync(string $Table, KeyedContainer<string, string> $Arguments):int
  public async function update(string $Table, KeyedContainer<string, string> $Where, KeyedContainer<string, string> $ToUpdate):Awaitable<int>
  public function updateSync(string $Table, KeyedContainer<string, string> $Where, KeyedContainer<string, string> $ToUpdate):int
  public async function delete(string $Table, KeyedContainer<string, string> $Where, ?int $Limit = null):Awaitable<int>
  public function deleteSync(string $Table, KeyedContainer<string, string> $Where, ?int $Limit = null):int
  public async function exists(string $Table, KeyedContainer<string, string> $Where):Awaitable<bool>
  public function existsSync(string $Table, KeyedContainer<string, string> $Where):bool
}
```

#### LICENSE
This project is licensed under the terms of MIT License. See the LICENSE file for more info.

[HHVM]:http://hhvm.com
[HackLang]:http://hacklang.org
