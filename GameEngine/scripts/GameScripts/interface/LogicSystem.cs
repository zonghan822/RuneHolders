namespace Eno
{
    /// <summary>
    /// Base class for game-level systems driven from C#.
    /// Unlike entity scripts, a LogicSystem is attached to a
    /// dedicated non-visual entity and manages global game state.
    /// </summary>
    public abstract class LogicSystem : ScriptBase { }
}
