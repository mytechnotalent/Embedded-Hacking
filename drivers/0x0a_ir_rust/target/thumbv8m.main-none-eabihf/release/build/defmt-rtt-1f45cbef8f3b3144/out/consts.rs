/// RTT buffer size (default: 1024).
            ///
            /// Can be customized by setting the `DEFMT_RTT_BUFFER_SIZE` environment variable.
            /// Use a power of 2 for best performance.
            pub(crate) const BUF_SIZE: usize = 1024;