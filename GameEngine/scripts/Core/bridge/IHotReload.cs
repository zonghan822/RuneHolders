public interface IHotReloadManager
{
    void Init(string dll);
    void LoadTemp();
    void Load(string path);
    string? CompileToTemp(string usPath, string dllPath);
    void Unload();
}