$ErrorActionPreference = "Stop"

$pipeName = "GeoIPSVCv1"
$length = 1000

# Generate an array of $length random bytes
$randomBytes = [byte[]]::new($length)
$random = [System.Security.Cryptography.RandomNumberGenerator]::Create()
$random.GetBytes($randomBytes)

# Create a named pipe client and connect to the named pipe
$pipeClient = New-Object System.IO.Pipes.NamedPipeClientStream(".", $pipeName, [System.IO.Pipes.PipeDirection]::InOut)
$pipeClient.Connect(5000)

try {
    # Send the random bytes through the named pipe
    $pipeWriter = New-Object System.IO.StreamWriter($pipeClient)
    $pipeWriter.BaseStream.Write($randomBytes, 0, $randomBytes.Length)
    $pipeWriter.Flush()
    $pipeWriter.Dispose()
} finally {
    # Disconnect from the named pipe and clean up resources
    $pipeClient.Dispose()
}
