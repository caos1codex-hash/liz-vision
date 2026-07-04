#pragma once

/// Interface for sprint demo runners.
/// Each sprint implements this to keep demo logic out of main().
class DemoRunner {
public:
    virtual ~DemoRunner() = default;

    /// Execute the demo.  Return 0 on success.
    virtual int run() = 0;
};
