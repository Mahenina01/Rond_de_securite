/** @type {import('tailwindcss').Config} */
export default {
  content: ["./index.html", "./src/**/*.{js,ts,jsx,tsx}"],
  darkMode: "class",
  theme: {
    extend: {
      // Palette de couleurs
      colors: {
        primary: {
          DEFAULT: "#00A88F",
          hover: "#008F7A",
          light: "#007F70",
          outline: "#00A88F",
          waiting: "#008F7A",
        },

        secondary: {
          DEFAULT: "#64748B",
          hover: "#475569",
        },

        background: "rgb(var(--color-background) / <alpha-value>)",
        surface: "rgb(var(--color-surface) / <alpha-value>)",
        input: "rgb(var(--color-input) / <alpha-value>)",

        text: {
          primary: "rgb(var(--color-text-primary) / <alpha-value>)",
          secondary: "rgb(var(--color-text-secondary) / <alpha-value>)",
          muted: "rgb(var(--color-text-muted) / <alpha-value>)",
        },

        border: "rgb(var(--color-border) / <alpha-value>)",

        success: {
          DEFAULT: "#16A34A",
          light: "#DCFCE7",
        },

        danger: {
          DEFAULT: "#DC2626",
          light: "#FEE2E2",
        },

        warning: {
          DEFAULT: "#D97706",
          light: "#FEF3C7",
        },
      },

      // Typographie
      fontFamily: {
        sans: ["Inter", "sans-serif"],
        heading: ["Inter", "sans-serif"],
      },

      fontSize: {
        "2xs": ["0.6875rem", { lineHeight: "1rem" }],
      },

      // Arrondis
      borderRadius: {
        card: "1rem",
        button: "0.625rem",
        input: "0.5rem",
      },

      // Ombres
      boxShadow: {
        card: "0 2px 8px rgb(15 23 42 / 5%)",
        dropdown: "0 8px 24px rgb(15 23 42 / 10%)",
      },

      // Espacements personnalisés
      spacing: {
        section: "1.5rem",
      },
    },
  },
  plugins: [],
};
