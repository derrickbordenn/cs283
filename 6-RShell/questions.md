1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

_The remote client determines when a command's output is fully received from the server by checking for a special end-of-transmission marker character. This is implemented using the RDSH_EOF_CHAR constant defined as 0x04 aka the ASCII EOF character. When the client receives this character as the last byte in a response, it knows that the server has finished sending all output for that command. The client handles partial reads by using a loop to continuously receive data until the EOF character is detected. This is implemented in the exec_remote_cmd_loop_

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

_Since TCP is a stream protocol and doesn't preserve message boundaries, a networked shell protocol needs to implement its own message framing technique. The protocol should define a way to identify the beginning of a command (in this implementation, it's implicit in that each command starts after receiving the prompt) and a way to identify the end of a command's output (using the EOF character as shown in the code). Challenges that we might encounter if this isn't handled correctly include message fragmentation where single command might be split across multiple TCP packets, message combination where multiple commands might come in a single TCP packet, and confusion between command input and command output. A lock of correct framing framing might result in a client continuing to wait for more data indefinitely or possibly interpreting part of the next command's output as belonging to the current command.
_

3. Describe the general differences between stateful and stateless protocols.

_Stateful protocols maintain information about the client's session across multiple requests, while stateless protocols don't maintain any session information between requests: Stateful protocols maintain session state on the server, remember previous interactions, often use session IDs or tokens, and will usually give more context for each request. Stateless protocols don't maintain session state on the server, result in independent requests that contain all necessary information, and result in more scalablility and resilience to server failures. The remote shell protocol implemented in our code allows the server to maintain connection state and context for each client._

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

_UDP is considered unreliable since it doesn't always ensure a properly delivery, ordering, or protection against duplication of packets, but it's still valuable in certain scenarios. It allows for lower latency, higher output, and simpler implementation. Applications can implement their own reliability mechanisms on top of UDP when needed, keeping only the aspects of reliability that are important for their specific use case._

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

_The operating system provides the Socket API as the interface/abstraction for network communications which includes socket creation, connection establishment, data transfer, and connection termination that my program implements through all the socket calls from this library. The Socket API abstracts away the complexity of network protocols, allowing the application to focus on communication logic rather than lower level details. The implementation in my code uses these socket functions to create the client-server communication channel._
