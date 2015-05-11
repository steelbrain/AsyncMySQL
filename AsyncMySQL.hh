<?hh //strict
type AsyncDatabaseResult = shape('Count' => int, 'Rows' => Vector<Map<string, string>>, 'ID' => int);
class AsyncDatabaseClient{
  public static async function connect(string $Host, int $Port, string $Database, string $User, string $Password, int $Timeout = -1):Awaitable<AsyncDatabase>{
    $AsyncDB = await AsyncMysqlClient::connect($Host, $Port, $Database, $User, $Password, $Timeout);
    return new AsyncDatabase($AsyncDB);
  }
}
class AsyncDatabase{
  public function __construct(private AsyncMysqlConnection $Con){}
  public function query(string $Query, ImmMap<string, string> $Arguments):AsyncDatabaseResult{
    $AsyncQuery = $this->Con->query($this->ParseQuery($Query, $Arguments))->getWaitHandle()->join();
    return shape(
      'Count' => $AsyncQuery->numRows(),
      'Rows' => $AsyncQuery->mapRows(),
      'ID' => $AsyncQuery->lastInsertId()
    );
  }
  public async function aquery(string $Query, ImmMap<string, string> $Arguments):Awaitable<AsyncDatabaseResult>{
    $Query = $this->ParseQuery($Query, $Arguments);
    $AsyncQuery = await $this->Con->query($Query);
    return shape(
      'Count' => $AsyncQuery->numRows(),
      'Rows' => $AsyncQuery->mapRows(),
      'ID' => $AsyncQuery->lastInsertId()
    );
  }
  <<__Memoize>>
  private function ParseQuery(string $Query, ImmMap<string, string> $Arguments):string{
    $Con = $this->Con;
    return preg_replace_callback('/(:[\w0-9])+/', function($Match) use($Arguments, $Con) {
      if($Arguments->contains($Match[0])){
        return $Con->escapeString($Arguments[$Match[0]]);
      } else {
        return $Match[0];
      }
    }, $Query);
  }
}
