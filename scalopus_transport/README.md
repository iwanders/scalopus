# scalopus_transport

Two transports factories are provided out of the box.
- `TransportUnixFactory`: Uses abstract unix domain sockets, this are unix domain sockets without a file in the
  filesystem. Allows for discovering all other servers running on the same host.
- `TransportLoopbackFactory`: This transport only supports communication within the same process. This is helpful when
  all components are within the same process.

Both transports have a worker thread on both the client and the server side.

## TransportUnix

Binds an abstract unix domain socket by name using `getpid()_scalopus` as name. Discovery happens by scanning the 
`/proc/net/unix` file and scanning for non-client entries that end with `_scalopus`.

Data is serialized using a simple length prefixed protocol:

request_id  | endpoint_name_length | endpoint_name  | data_length  | data
------------|----------------------|----------------|--------------|---------------
std::size_t | std::uint16_t        | std::uint8_t[] | std::uin32_t | std::uint8_t[]

Each connection must sent a complete transmission before starting another, this means that the receiving end can keep
reading data from the socket until the transmission is completely read or until the connection disconnects.

## TransportLoopback

This transport just keeps a list of pointers of instantiated servers and allows discovery of those. No serialization is
needed as the data can directly be passed to the appropriate methods.
