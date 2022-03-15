# STUDENT ATTENDANCE MARKING DEVICE WITH BIOMETRIC AUTHENTICATION
==================================================================

- Controlled by an ARM Cortex-M4 (STM32F429ZIT) 
- Fingerprint sensor talks to the Cortex-M4 via UART. ([fingerprint.c](Src/fingerprint.c))
- Enclosed in a 3D-printed ergonomic enclosure.

![Picture4](https://user-images.githubusercontent.com/93194810/139519169-124b6dec-0a32-47c9-acff-7edaa8f35ce3.png)

![Picture5](https://user-images.githubusercontent.com/93194810/139519178-2d3f7f98-f5f4-47c5-9989-216c50d2219f.jpg)

- Powered by a Li-ion Battery

![PD](https://user-images.githubusercontent.com/93194810/139519209-ca74357d-36b0-4a60-903b-a0e562928576.jpg)


## Firmware Implementation

- Super-Loop Architecture :

![Screenshot 2021-10-30 145144](https://user-images.githubusercontent.com/93194810/139519284-0f6d3c37-f31e-4f5d-b59d-2944f931bf43.jpg)

- Cortex-M4 and User program interaction :

![4](https://user-images.githubusercontent.com/93194810/139519379-1ac173ed-b8ca-44ad-a448-050cd5fb5f7e.jpg)

- Identifying a Fingerprint : 

![Screenshot 2021-10-30 145320](https://user-images.githubusercontent.com/93194810/139519315-81244481-188d-4c5b-a810-fbbb809b9eae.jpg)

- Registering a Fingerpirnt : 

![3](https://user-images.githubusercontent.com/93194810/139519335-241b4d46-4e56-48aa-a761-d2d437b5a8c8.jpg)
