<?hh //strict
type AsyncDatabaseResult = shape('Count' => int, 'Rows' => Vector<Map<string, string>>, 'ID' => int, 'Affected' => int);
class AsyncDatabase{
  public function __construct(private AsyncMysqlConnection $Con){}
  public function querySync(string $Query, KeyedContainer<string, string> $Arguments):AsyncDatabaseResult{
    return $this->query($Query, $Arguments)->getWaitHandle()->join();
  }
  public function insertSync(string $Table, KeyedContainer<string, string> $Arguments):int{
    return $this->insert($Table, $Arguments)->getWaitHandle()->join();
  }
  public function deleteSync(string $Table, KeyedContainer<string, string> $Where, ?int $Limit = null):int{
    return $this->delete($Table, $Where, $Limit)->getWaitHandle()->join();
  }
  public function existsSync(string $Table, KeyedContainer<string, string> $Where):bool{
    return $this->exists($Table, $Where)->getWaitHandle()->join();
  }
  public function updateSync(string $Table, KeyedContainer<string, string> $Where, KeyedContainer<string, string> $ToUpdate):int{
    return $this->update($Table, $Where, $ToUpdate)->getWaitHandle()->join();
  }
  public async function query(string $Query, KeyedContainer<string, string> $Arguments):Awaitable<AsyncDatabaseResult>{
    $Query = $this->ParseQuery($Query, $Arguments);
    $AsyncQuery = await $this->Con->query($Query);
    return shape(
      'Count' => $AsyncQuery->numRows(),
      'Rows' => $AsyncQuery->mapRows(),
      'ID' => $AsyncQuery->lastInsertId(),
      'Affected' => $AsyncQuery->numRowsAffected()
    );
  }
  public async function insert(string $Table, KeyedContainer<string, string> $Arguments):Awaitable<int>{
    $Keys = Vector{};
    $Placeholders = Vector{};
    $Values = Map{};
    foreach($Arguments as $Key => $Value){
      $Keys->add($Key);
      $Placeholders->add(':insert_'.$Key);
      $Values->set(':insert_'.$Key, $Value);
    }
    $Keys = implode(', ', $Keys);
    $Placeholders = implode(', ', $Placeholders);
    $Query = "Insert INTO {$Table} ($Keys) VALUES ($Placeholders)";
    $Query = await $this->query($Query, $Values);
    return $Query['ID'];
  }
  public async function update(string $Table, KeyedContainer<string, string> $Where, KeyedContainer<string, string> $ToUpdate):Awaitable<int>{
    $Where = $this->ParseWhere($Where);
    $Arguments = $Where[1];
    $Where = $Where[0];
    $Update = Vector{};
    foreach($ToUpdate as $Key => $Value){
      $Update->add("$Key = :update_{$Key}");
      $Arguments->set(":update_{$Key}", $Value);
    }
    $Update = implode(', ', $Update);
    $Query = "Update $Table SET $Update $Where LIMIT 1";
    $Query = await $this->query($Query, $Arguments);
    return $Query['Affected'];
  }
  public async function delete(string $Table, KeyedContainer<string, string> $Where, ?int $Limit = null):Awaitable<int>{
    if($Limit === null){
      $Limit = 1;
    }
    $Where = $this->ParseWhere($Where);
    $Query = "Delete from $Table ".$Where[0]." LIMIT ".$Limit;
    $Query = await $this->query($Query, $Where[1]);
    return $Query['Affected'];
  }
  public async function exists(string $Table, KeyedContainer<string, string> $Where):Awaitable<bool>{
    $Where = $this->ParseWhere($Where);
    $Query = "Select 1 from $Table ".$Where[0]." LIMIT 1";
    $Query = await $this->query($Query, $Where[1]);
    return (bool) $Query['Count'];
  }
  <<__Memoize>>
  private function ParseWhere(KeyedContainer<string, string> $Where):(string, Map<string, string>){
    $Arguments = Map{};
    $Query = Vector{};
    foreach($Where as $Key => $Value){
      $Query->add("$Key = :where_{$Key} ");
      $Arguments->set(':where_'.$Key, $Value);
    }
    if($Query->count()){
      $Query = ' WHERE '.implode('AND ', $Query);
    } else {
      $Query = '';
    }
    return tuple($Query, $Arguments);
  }
  <<__Memoize>>
  private function ParseQuery(string $Query, KeyedContainer<string, string> $Arguments):string{
    return preg_replace_callback('/(:[\w0-9]+)/', function($Match) use($Arguments) {
      if(array_key_exists($Match[0], $Arguments)){
        return '"'.$this->Con->escapeString($Arguments[$Match[0]]).'"';
      } else {
        return $Match[0];
      }
    }, $Query);
  }
  public static async function connect(string $Host, int $Port, string $Database, string $User, string $Password, int $Timeout = -1):Awaitable<AsyncDatabase>{
    $AsyncDB = await AsyncMysqlClient::connect($Host, $Port, $Database, $User, $Password, $Timeout);
    return new AsyncDatabase($AsyncDB);
  }
}
