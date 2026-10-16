import * as XLSX from "xlsx";
import { jsPDF } from "jspdf";
import autoTable from "jspdf-autotable";

/**
 * Convertit une image (URL ou chemin public) en Data URL
 * pour pouvoir l'insérer dans un PDF.
 */
const loadImageAsDataURL = async (url) => {
  if (!url) return null;

  try {
    const response = await fetch(url);
    if (!response.ok) {
      throw new Error(`Impossible de charger le logo : ${url}`);
    }

    const blob = await response.blob();

    return await new Promise((resolve, reject) => {
      const reader = new FileReader();
      reader.onload = () => resolve(reader.result);
      reader.onerror = reject;
      reader.readAsDataURL(blob);
    });
  } catch (error) {
    console.error("Erreur de chargement du logo :", error);
    return null;
  }
};

/**
 * Normalise les données destinées à l'exportation.
 *
 * columns : [
 *   { header: "Identifiant", dataKey: "id" },
 *   { header: "Nom", dataKey: "nom" }
 * ]
 */
const prepareData = (data, columns) => {
  if (!Array.isArray(data)) {
    throw new Error("Les données doivent être un tableau.");
  }

  const keys =
    columns?.map((column) => column.dataKey) ?? Object.keys(data[0] ?? {});

  return {
    keys,
    headers: columns?.map((column) => column.header) ?? keys,
    rows: data.map((item) =>
      keys.map((key, index) => {
        const column = columns?.[index];
        const value = item[key];

        return column?.formatter
          ? column.formatter(value, item)
          : (value ?? "");
      }),
    ),
  };
};

/**
 * Sécurise une cellule CSV.
 */
const escapeCSV = (value) => {
  let text = String(value ?? "");

  // Évite l'exécution de formules dans les tableurs.
  if (/^[\s]*[=+\-@]/.test(text)) {
    text = `'${text}`;
  }

  return `"${text.replace(/"/g, '""')}"`;
};

/**
 * Exportation CSV.
 */
const exportCSV = (data, columns, filename) => {
  const { headers, rows } = prepareData(data, columns);

  const csvContent = [
    headers.map(escapeCSV).join(";"),
    ...rows.map((row) => row.map(escapeCSV).join(";")),
  ].join("\r\n");

  // BOM pour améliorer la reconnaissance de l'UTF-8 par Excel.
  const blob = new Blob(["\uFEFF", csvContent], {
    type: "text/csv;charset=utf-8;",
  });

  downloadBlob(blob, `${filename}.csv`);
};

/**
 * Exportation Excel.
 */
const exportExcel = (data, columns, filename, title) => {
  const { headers, rows } = prepareData(data, columns);

  const worksheet = XLSX.utils.aoa_to_sheet([[title], [], headers, ...rows]);

  worksheet["!cols"] = headers.map((header, index) => ({
    wch: Math.min(
      Math.max(
        String(header).length + 4,
        ...rows.map((row) => String(row[index] ?? "").length + 2),
      ),
      35,
    ),
  }));

  const workbook = XLSX.utils.book_new();
  XLSX.utils.book_append_sheet(workbook, worksheet, "Rapport");
  XLSX.writeFile(workbook, `${filename}.xlsx`);
};

/**
 * Exportation PDF avec logo, titre, description et tableau.
 */
const exportPDF = async (
  data,
  columns,
  filename,
  { title, description, logo },
) => {
  const { headers, rows } = prepareData(data, columns);
  const pdf = new jsPDF({
    orientation: headers.length > 5 ? "landscape" : "portrait",
    unit: "mm",
    format: "a4",
  });

  const pageWidth = pdf.internal.pageSize.getWidth();
  const margin = 14;
  let startY = 18;

  // Logo : par exemple /assets/logo.png (dans public/assets).
  if (logo) {
    const logoData = await loadImageAsDataURL(logo);

    if (logoData) {
      try {
        const format = logoData.includes("image/png")
          ? "PNG"
          : logoData.includes("image/webp")
            ? "WEBP"
            : "JPEG";

        pdf.addImage(logoData, format, margin, 10, 22, 22);
        startY = 20;
      } catch (error) {
        console.error("Impossible d'insérer le logo :", error);
      }
    }
  }

  // Titre et description.
  const titleX = logo ? 42 : margin;

  pdf.setFont("helvetica", "bold");
  pdf.setFontSize(18);
  pdf.text(title, titleX, startY + 3, {
    maxWidth: pageWidth - titleX - margin,
  });

  pdf.setFont("helvetica", "normal");
  pdf.setFontSize(10);

  const descriptionLines = pdf.splitTextToSize(
    description || "",
    pageWidth - margin * 2,
  );

  const descriptionY = startY + 11;

  if (descriptionLines.length) {
    pdf.text(descriptionLines, margin, descriptionY);
  }

  const tableStartY = Math.max(
    logo ? 34 : 20,
    descriptionY + descriptionLines.length * 5 + 6,
  );

  autoTable(pdf, {
    startY: tableStartY,
    head: [headers],
    body: rows,
    margin: { top: 15, right: margin, bottom: 18, left: margin },
    styles: {
      fontSize: headers.length > 6 ? 7 : 9,
      cellPadding: 3,
      overflow: "linebreak",
    },
    headStyles: {
      fillColor: [35, 11, 98],
      textColor: [255, 255, 255],
      fontStyle: "bold",
    },
    alternateRowStyles: {
      fillColor: [245, 245, 250],
    },
    didDrawPage: () => {
      const pageHeight = pdf.internal.pageSize.getHeight();
      const pageCount = pdf.internal.getNumberOfPages();

      pdf.setFontSize(8);
      pdf.setFont("helvetica", "normal");
      pdf.text(
        `Document généré le ${new Date().toLocaleDateString("fr-FR")}`,
        margin,
        pageHeight - 8,
      );
      pdf.text(
        `Page ${pdf.internal.getCurrentPageInfo().pageNumber}/${pageCount}`,
        pageWidth - margin,
        pageHeight - 8,
        { align: "right" },
      );
    },
  });

  pdf.save(`${filename}.pdf`);
};

/**
 * Téléchargement d'un Blob.
 */
const downloadBlob = (blob, filename) => {
  const url = URL.createObjectURL(blob);
  const link = document.createElement("a");

  link.href = url;
  link.download = filename;
  document.body.appendChild(link);
  link.click();
  link.remove();

  URL.revokeObjectURL(url);
};

/**
 * API publique réutilisable.
 *
 * format : "csv" | "xlsx" | "pdf"
 */
export const exportData = async ({
  data,
  columns,
  format,
  filename = "export",
  title = "Rapport",
  description = "",
  logo = null,
}) => {
  if (!Array.isArray(data) || data.length === 0) {
    throw new Error("Aucune donnée à exporter.");
  }

  const options = {
    data,
    columns,
    filename,
    title,
    description,
    logo,
  };

  switch (format.toLowerCase()) {
    case "csv":
      exportCSV(options.data, options.columns, options.filename);
      break;

    case "xlsx":
    case "excel":
      exportExcel(
        options.data,
        options.columns,
        options.filename,
        options.title,
      );
      break;

    case "pdf":
      await exportPDF(options.data, options.columns, options.filename, options);
      break;

    default:
      throw new Error("Format non pris en charge. Utilisez csv, xlsx ou pdf.");
  }
};
