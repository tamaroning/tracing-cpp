#include "tracing.h"

int main() {
  tracing::Builder builder = tracing::Builder::from_env("CPP_LOG");
  builder.init();

  tracing::info("Hello, {}!", "world");
  tracing::debug("1 + 1 = {}", 1 + 1);
}
