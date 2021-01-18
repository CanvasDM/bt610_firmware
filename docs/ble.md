# BLE Services and information used in the BT610 OOB Demo

## Advertisement

The advertisement includes the UUID of the Cellular Profile. The complete local name is included in the scan response.
The complete local name is "MG100 OOB-1234567", where "1234567" are replaced with the last 7 digits of the IMEI.

## SMP Service

### UUID: 8D53DC1D-1DB7-4CD3-868B-8A527460AA84

Characteristics:

| Name                                | UUID                                 | Properties | Description                                                                                                                                                                                                                                                                                                                        |
| ----------------------------------- | ------------------------------------ | ---------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| SMP                  | da2e7828-fbce-4e01-ae9e-261174997c48 | read/write       | The Group Id is equal to 65. Usering key-value pair. 