from smolagents import CodeAgent, tool, InferenceClientModel

from huggingface_hub import login
login()

# 根据场合建议菜单的工具
@tool
def suggest_menu(occasion: str) -> str:
    """
    Suggests a menu based on the occasion.
    Args:
        occasion: The type of occasion for the party.
    """
    if occasion == "casual":
        return "Pizza, snacks, and drinks."
    elif occasion == "formal":
        return "3-course dinner with wine and dessert."
    elif occasion == "superhero":
        return "Buffet with high-energy and healthy food."
    else:
        return "Custom menu for the butler."

# 管家阿尔弗雷德，为派对准备菜单
agent = CodeAgent(tools=[suggest_menu], model=InferenceClientModel())

# 为派对准备正式菜单
agent.run("Prepare a formal menu for the party.")
