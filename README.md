# Arcweave Plugin example project
Example demo project which demonstrates the Arcweave > Unreal integration

## Project setup
### 1. Clone or Download the Project
Choose **one** of the two methods below to get the project files.

<details>
<summary> Option A: Using Git (Recommended) </summary>
This is the fastest method as it automatically includes the necessary plugin as a submodule. Run the following command in your terminal:

```bash
git clone --recurse-submodules [https://github.com/arcweave/arcweave-unreal-example](https://github.com/arcweave/arcweave-unreal-example)
```
</details>

<details>
<summary>Option B: Manual Download (No Git) </summary>
If you prefer not to use Git, you must download the project and the plugin separately:

1. **Download the Project:** [Download Project ZIP](https://github.com/arcweave/arcweave-unreal-example/archive/refs/heads/main.zip) and unzip it.
2. **Download the Plugin:** [Download Plugin ZIP](https://github.com/arcweave/arcweave-unreal-plugin/archive/refs/heads/main.zip) and unzip it.
3. **Install the Plugin:** Move the unzipped plugin folder into the project directory at:  
   `ArcweaveDemo/Plugins/`

> [!TIP]
> If the `Plugins` folder doesn't exist yet inside `ArcweaveDemo`, simply create a new folder and name it `Plugins`.
</details>

### 2. Build & Launch

1. **Generate Files:** Right-click `ArcweaveDemo.uproject` → **Generate Visual Studio project files**.
2. **Prerequisites:** Ensure [Visual Studio 2022](https://www.youtube.com/watch?v=8xJRr6Yr_LU) and its C++ dependencies are installed.
3. **Compile:**
   * Open the `.sln` file.
   * Right-click **Games > ArcweaveDemo** and select **Build**.
4. **Open:** Once the build finishes, close Visual Studio and double-click `ArcweaveDemo.uproject`.

## Git submodule tips
This repo contains the submodule of the unreal arcweave plugin

1. Initialization: If you don't see anything in the Plugins folder or it is missing run ```git submodule update --init --recursive``` to actually see the files inside Plugin.
2. Detached HEAD: When you enter a submodule, you are usually in a "Detached HEAD" state. If you make changes there, remember to checkout a branch (like main) before committing.

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
