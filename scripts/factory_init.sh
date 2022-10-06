#!/zsh

# Setup DM connection basics
attr set lwm2m_security 3
attr set lwm2m_bootstrap 1

# Generate private keys
pki keygen p2p