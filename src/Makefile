BUILD_DIR := build
OUT_DIR   := out
PWSH      := powershell.exe


.PHONY: all clean run serve

ifeq ($(OS),Windows_NT)

all:
	@$(PWSH) "if (!(Test-Path '$(BUILD_DIR)')) { New-Item -ItemType Directory -Path '$(BUILD_DIR)' }"
	@$(PWSH) "if (!(Test-Path '$(OUT_DIR)'))   { New-Item -ItemType Directory -Path '$(OUT_DIR)' }"
	@$(PWSH) "cd '$(BUILD_DIR)'; cmake .."
	@$(PWSH) "cd '$(BUILD_DIR)'; cmake --build . --config Release"

run:
	@$(PWSH) "Copy-Item 'lib\\*.dll' '$(OUT_DIR)' -Force"
	@$(PWSH) "Copy-Item 'model_list.json' '$(OUT_DIR)' -Force"
	@$(PWSH) "cd '$(OUT_DIR)'; .\\flm.exe run llama3.2:1b"

serve:
	@$(PWSH) "Copy-Item 'lib\\*.dll' '$(OUT_DIR)' -Force"
	@$(PWSH) "Copy-Item 'model_list.json' '$(OUT_DIR)' -Force"
	@$(PWSH) "cd '$(OUT_DIR)'; .\\flm.exe serve llama3.2:1b"

clean:
	@$(PWSH) "Remove-Item -Recurse -Force '$(BUILD_DIR)'"
	@$(PWSH) "Remove-Item -Recurse -Force '$(OUT_DIR)'"

else

all:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OUT_DIR)
	@cd $(BUILD_DIR) && ${PWSH} -Command "cmake .."
	@cd $(BUILD_DIR) && ${PWSH} -Command "cmake --build . --config Release"

run:
	@cp lib/*.dll $(OUT_DIR)
	@cp model_list.json $(OUT_DIR)
	@cd $(OUT_DIR) && ${PWSH} -Command "./flm.exe run llama3.2:1b"

serve:
	@cp lib/*.dll $(OUT_DIR)
	@cp model_list.json $(OUT_DIR)
	@cd $(OUT_DIR) && ${PWSH} -Command "./flm.exe serve llama3.2:1b"

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(OUT_DIR)

endif
