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
$Query = await $DBAsync->aquery("Update Users set Plan = :plan, Status = :status, FirstName = :fname, LastName = :lname, LastActive = :lactive, Server = (Select ID from Servers where SecretToken = :token) where ID = :id LIMIT 1",
   ImmMap{
    ':plan' => (int) $Processed['plan'],
    ':status' => (int) $Processed['status'],
    ':fname' => $Processed['firstname'],
    ':lname' => $Processed['lastname'],
    ':lactive' => (int) $Processed['lastactive'],
    ':token' => (int) $Processed['token'],
    ':id' => (int) $Processed['id']
});
```

#### API
```hack
class AsyncDatabaseClient{
  public static async function connect(string $Host, int $Port, string $Database, string $User, string $Password, int $Timeout = -1):Awaitable<AsyncDatabase>
}
class AsyncDatabase{
  public function __construct(private AsyncMysqlConnection $Con)
  public function query(string $Query, ImmMap<string, string> $Arguments):AsyncDatabaseResult
  public async function aquery(string $Query, ImmMap<string, string> $Arguments):Awaitable<AsyncDatabaseResult>
}
```

#### LICENSE
This project is licensed under the terms of MIT License. See the LICENSE file for more info.

[HHVM]:http://hhvm.com
[HackLang]:http://hacklang.org