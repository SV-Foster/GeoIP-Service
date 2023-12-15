$ErrorActionPreference = "Stop"

# Define the target IP address and port
$ipAddress = "127.0.0.1"
$port = "28780"
$length = 64
$times = 10

# Generate an array of random bytes
$randomBytes = [byte[]]::new($length)
$random = [System.Security.Cryptography.RandomNumberGenerator]::Create()
$random.GetBytes($randomBytes)

# Create a TCP client and connect to the target IP and port
$client = New-Object System.Net.Sockets.TcpClient
$client.Connect($ipAddress, $port)

try {
    # Create a network stream for sending data
    $stream = $client.GetStream()
    
    # Send the random bytes array to the target server
    For ($i=1; $i -le $times; $i++) {
    $i
        $stream.Write($randomBytes, 0, $randomBytes.Length)
        sleep(1)
    }
    Write-Host "Sent $length random bytes to $ipAddress on port $port $times times"
} catch {
    Write-Host "Failed to send bytes to $ipAddress on port $port. Error: $_"
} finally {
    # Close the TCP client
    $client.Close()
}
