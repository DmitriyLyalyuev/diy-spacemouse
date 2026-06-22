import adsk.core, adsk.fusion, adsk.cam, traceback
import os

COMMAND_ID = 'HomeViewButton'
COMMAND_NAME = 'Send To Home View'
COMMAND_DESCRIPTION = 'Send the active viewport to the default Home View. Bind this command to Cmd+Shift+N.'
PANEL_ID = 'SolidScriptsAddinsPanel'
RESOURCES_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'resources', '')

# Global list to keep all event handlers in scope.
# This is only needed with Python.
handlers = []


def run(context):
    ui = None
    try:
        app = adsk.core.Application.get()
        ui = app.userInterface

        cmd_defs = ui.commandDefinitions

        home_view_button = cmd_defs.addButtonDefinition(
            COMMAND_ID,
            COMMAND_NAME,
            COMMAND_DESCRIPTION,
            RESOURCES_DIR
        )

        home_view_command_created = HomeViewCommandCreatedHandler()
        home_view_button.commandCreated.add(home_view_command_created)
        handlers.append(home_view_command_created)

        addins_panel = ui.allToolbarPanels.itemById(PANEL_ID)
        if addins_panel:
            addins_panel.controls.addCommand(home_view_button)
        else:
            ui.messageBox(
                'Send Home command was created, but the ADD-INS toolbar panel was not found. '
                'You can still assign a keyboard shortcut from Fusion 360 command settings.'
            )
    except:
        if ui:
            ui.messageBox('Failed:\n{}'.format(traceback.format_exc()))


# Event handler for the commandCreated event.
class HomeViewCommandCreatedHandler(adsk.core.CommandCreatedEventHandler):
    def __init__(self):
        super().__init__()

    def notify(self, args):
        event_args = adsk.core.CommandCreatedEventArgs.cast(args)
        cmd = event_args.command

        # Connect to the execute event.
        on_execute = HomeViewCommandExecuteHandler()
        cmd.execute.add(on_execute)
        handlers.append(on_execute)


# Event handler for the execute event.
class HomeViewCommandExecuteHandler(adsk.core.CommandEventHandler):
    def __init__(self):
        super().__init__()

    def notify(self, args):
        app = adsk.core.Application.get()

        vp: adsk.core.Viewport = app.activeViewport
        vp.goHome(True)
        vp.refresh()


def stop(context):
    ui = None
    try:
        app = adsk.core.Application.get()
        ui = app.userInterface

        cmd_def = ui.commandDefinitions.itemById(COMMAND_ID)
        if cmd_def:
            cmd_def.deleteMe()

        addins_panel = ui.allToolbarPanels.itemById(PANEL_ID)
        if addins_panel:
            cntrl = addins_panel.controls.itemById(COMMAND_ID)
            if cntrl:
                cntrl.deleteMe()
    except:
        if ui:
            ui.messageBox('Failed:\n{}'.format(traceback.format_exc()))
