# Traceroute
A program that display the IP addresses of routers on the path to a destination IP address using raw sockets in Linux operating system.

## Compilation:
Using the Makefile. Simply type `make` in terminal.

## Example of use:
`
sudo ./traceroute 8.8.8.8
`

## Output:
The program displays IP addresses and average connection time.
If it's not possible to get the IP address of router, the program displays `*` instead.