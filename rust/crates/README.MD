# Vendorized Crates

This directory contains vendorized crates that we source internally.

**Why We Vendorize Crates**

* **Enhanced Security:** Internal repositories offer greater control over code quality, vulnerability scanning, and the ability to promptly address security flaws.
* **Compatibility and Version Control:** Vendorization ensures seamless integration within our AOSP environment, minimizing conflicts that could arise from external dependencies.
* **Self contained:** A private repository tailored to our needs requires no external downloads during development.

**Obtaining Crates**

* **AOSP:** Available at [https://android.googlesource.com/platform/external/rust/crates/](https://android.googlesource.com/platform/external/rust/crates/). Please note: these crates lack `.cargo-checksum.json` files. You'll need to patch them into your workspace definition.

**Example: Using bindgen in AOSP**

```rust
[patch.crates-io]
bindgen = { path = "../../../../../external/rust/crates/bindgen" }
```

## Adding External Crates (Strongly Discouraged)

If you must bring in an external crate, be aware:

* **Ownership:** You'll be responsible for maintenance, including bug fixes and security patches.
* **Review:** A strict review process is required see details [here](https://g3doc.corp.google.com/company/thirdparty/android.md).
* **Prioritize Internal Crates:**  The maintenance overhead of external crates can be substantial. We only use a limited set for Windows compatibility.
