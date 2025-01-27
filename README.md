# Arcweave Plugin example project
Example demo project which demonstrates the Arcweave > Unreal integration

## Project setup
1. Download the project to your local computer
2. Unzip the file
3. **Right-click -> ArcweaveDemo.uproject -> Generate visual studio files**
4. Make sure you have installed Visual Studio 2022 and all the dependencies [Tutorial](https://www.youtube.com/watch?v=8xJRr6Yr_LU).
5. Inside the Visual Studio solution right-click Games -> ArcweaveDemo -> Build solution
6. When the build is finished, close the Visual Studio and double-click the **ArcweaveDemo.uproject**

## Important classes
The entire logic, assets, and implementation of this demo are organized and stored in the **Content->ArcweaveDemo** directory. Within this directory, the most crucial class to understand is **WBP_HUD_New**, which is located in **Content\ArcweaveDemo\Widgets**. 
This class provides a comprehensive setup and detailed examples of how to:

- Fetch Data:
Demonstrates the process of retrieving data from the relevant source or API.

- Preserve User API Tokens and Hash for Packaging:
Outlines the steps to securely store and manage user API tokens and hash values, ensuring they are preserved during the packaging process.

- Extract Data:
Shows how to extract and process the data once it has been fetched.

- Transpile Elements, Objects, Conditions, etc.:
Provides examples and guidance on how to transpile various elements, objects, and conditions as required.

By exploring the **WBP_HUD_New** class and its implementations, you will gain valuable insights into the project's logic and workflow, serving as a practical guide for your own implementations.

## Data Collection Methods
There are two primary methods for collecting data:

**Web API:** This involves fetching data directly from the Arcweave Web API.

**JSON Import:** For this method, you'll need to import a JSON file from your Arcweave project. Ensure that the JSON file, along with all required assets, is placed in the **Content->ArcweaveExport directory**. If this directory does not exist, you will need to create it.
Also make sure you have enabled to receive data from local JSON in UE Settings > Project Settings > Plugins > Arcweave and you have restarted your UE for the setting to take effect.
