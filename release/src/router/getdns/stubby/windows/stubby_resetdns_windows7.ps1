#Requires -RunAsAdministrator
#Requires -Version 2

#Get enabled/connected adapters (same as 'Get-NetAdapter -Physical')
$NetworkAdapters = Get-WmiObject -Class 'Win32_NetworkAdapterConfiguration' -Filter {IPEnabled = 1}

#Verbose output so the user gets to know the current configuration
Write-Output -InputObject 'Found Adapters:'
Write-Output -InputObject $NetworkAdapters | Format-Table -Property IPAddress,DefaultIPGateway,DNSServerSearchOrder,Description

Write-Output -InputObject 'Resetting DNS servers - the system will use default DNS service..'

#Change the DNS entry for each found network adapter
foreach ($NetworkAdapter in $NetworkAdapters) {
  $result = $NetworkAdapter.SetDNSServerSearchOrder()
}

#Get enabled/connected adapters (same as 'Get-NetAdapter -Physical')
$NetworkAdapters = Get-WmiObject -Class 'Win32_NetworkAdapterConfiguration' -Filter {IPEnabled = 1}
Write-Output -InputObject 'Updated Adapters:'
Write-Output -InputObject $NetworkAdapters | Format-Table -Property IPAddress,DefaultIPGateway,DNSServerSearchOrder,Description

Write-Output -InputObject ''
Write-Output -InputObject '##############################  WARNING  ##############################'
Write-Output -InputObject 'Warning: this script can only update IPv4 addresses for DNS servers.'
Write-Output -InputObject 'Queries sent over IPv6 will still use the default DNS servers and'
Write-Output -InputObject 'send DNS queries in clear text. You may want to disable IPv6!!!'
Write-Output -InputObject '##############################  WARNING  ##############################'
