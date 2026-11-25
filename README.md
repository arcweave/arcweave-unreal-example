# Arcweave Plugin example project
Example demo project which demonstrates the Arcweave > Unreal integration

## Project setup
### 1. Clone or Download the Project
Choose **one** of the two methods below to get the project files.

<details>
<summary> Option A: Using Git (Recommended) </summary>
This is the fastest method as it automatically includes the necessary plugin as a submodule. Run the following command in your terminal:

```bash
git clone --recurse-submodules https://github.com/arcweave/arcweave-unreal-example.git
```
</details>

<details>
<summary>Option B: Manual Download (No Git) </summary>
If you prefer not to use Git, you must download the project and the plugin separately:

1. **Download the Project:** [Download Project ZIP](https://github.com/arcweave/arcweave-unreal-example/archive/refs/heads/main.zip) and unzip it.
2. **Download the Plugin:** [Download Plugin ZIP](https://github.com/arcweave/arcweave-unreal-plugin/archive/refs/heads/main.zip) and unzip it.
3. **Install the Plugin:** Create and move the unzipped plugin folder into the project directory at:  
   `ArcweaveDemo/Plugins/Arcweave`

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
The Arcweave plugin is stored in the project as a git submodule.

1. **Initialization:** If you don't see the plugin in the `ArcweaveDemo/Plugins` folder, run the following command to download the plugin files:

   ```bash
   git submodule update --init --recursive
   ```

2. **Detached HEAD:** When you enter a submodule, you are usually in a "Detached HEAD" state. If you make changes there, remember to checkout a branch (like `main`) before committing.

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

## Multiple languages
The icon on the top right of the HUD opens a menu to switch the language of the project at runtime. It uses the languages (locales) of your Arcweave project, so it depends on how the data is collected:

- **JSON Import:** export the project with **all languages**, so the JSON contains the `locales` and `contents` fields. Keep only **one** JSON file in `Content/ArcweaveExport`: the plugin loads the first one it finds, so replace the existing export instead of adding a second file. The language can then be changed at runtime with the menu. If the JSON has a single language, the menu shows a "No locales available" message.
- **Web API:** the API returns only one language per request, so the language can't be changed at runtime. Choose it in UE Settings > Project Settings > Plugins > Arcweave: enable **Use Locale**, set the **Locale** (e.g. `en`, `it`, `fr`) and, if you want, **Fallback To Default Locale** (it can only be edited once **Use Locale** is enabled). Restart the import to apply it.

A few things to know:

- The language chosen with the menu is only kept until the project is imported again, then the language set in the Project Settings is used. It never changes the Project Settings.
- A missing translation uses the base language of the locale as defined in Arcweave when **Fallback To Default Locale** is enabled, otherwise the text is empty.
- Switching to a language the project doesn't have fails and changes nothing: `UpdateContentsWithLocale` returns `false`.
- To react to a change of language (e.g. to refresh the UI), bind to the `OnArcweaveLanguageChanged` event of the Arcweave subsystem.

### The languages of the example project
`Content/ArcweaveExport/project_settings.json` is exported with all languages, so the language menu works out of the box. It contains two languages:

| Language | Code | Notes |
|---|---|---|
| English | `en` | The base language, every text is translated. |
| Greek | `el` | Based on English. Most dialogue lines, choices, titles and component names are translated. |

A few Greek texts are left untranslated on purpose, to show how the fallback works: the dialogue starts that contain script blocks, the "Go to ..." choices that mention a component and some of the notes.

Select the language with the icon on the top right of the HUD, or set **Locale** to `el` in UE Settings > Project Settings > Plugins > Arcweave.

#### What to expect when changing language
The result for a text that is not translated depends on **Fallback To Default Locale** in the Arcweave settings:

| Fallback To Default Locale | Greek text with a translation | Greek text without a translation |
|---|---|---|
| **Off** | Shown in Greek. | Empty: nothing is shown. |
| **On** | Shown in Greek. | Shown in English, the base language of Greek. |

- With the fallback **on**, switching to Greek gives a mostly Greek dialogue, and the few untranslated lines stay in English.
- With the fallback **off**, the untranslated lines are blank, so you can see exactly what is missing in a translation.
- Switching back to English always shows every text.
- The fallback only follows the base language set in Arcweave. A text is never taken from an unrelated language.
